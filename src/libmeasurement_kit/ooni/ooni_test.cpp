// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include "../ooni/utils.hpp"

namespace mk {
namespace ooni {

void OoniTest::run_next_measurement(size_t thread_id, Callback<Error> cb,
                                    size_t num_entries,
                                    Var<size_t> current_entry) {
    logger->debug("net_test: running next measurement");
    std::string next_input;
    std::getline(*input_generator, next_input);

    if (input_generator->eof()) {
        logger->debug("net_test: reached end of input");
        cb(NoError());
        return;
    }
    if (!input_generator->good()) {
        logger->warn("net_test: I/O error reading input");
        cb(FileIoError());
        return;
    }

    double prog = 0.0;
    if (num_entries > 0) {
        prog = *current_entry / (double)num_entries;
    }
    *current_entry += 1;
    logger->log(MK_LOG_INFO|MK_LOG_JSON, "{\"progress\": %f}", prog);

    logger->debug("net_test: creating entry");
    struct tm measurement_start_time;
    double start_time;
    mk::utc_time_now(&measurement_start_time);
    start_time = mk::time_now();

    logger->debug("net_test: calling setup");
    setup(next_input);

    logger->debug("net_test: running with input %s", next_input.c_str());
    main(next_input, options, [=](report::Entry test_keys) {
        report::Entry entry;
        entry["test_keys"] = test_keys;
        entry["test_keys"]["client_resolver"] = resolver_ip;
        entry["input"] = next_input;
        entry["measurement_start_time"] =
            *mk::timestamp(&measurement_start_time);
        entry["test_runtime"] = mk::time_now() - start_time;

        logger->debug("net_test: tearing down");
        teardown(next_input);

        report.fill_entry(entry);
        if (entry_cb) {
            entry_cb(entry.dump());
        }
        report.write_entry(entry, [=](Error error) {
            if (error) {
                cb(error);
                return;
            }
            logger->debug("net_test: written entry");
            reactor->call_soon([=]() {
                run_next_measurement(thread_id, cb, num_entries, current_entry);
            });
        });
    });
}

void OoniTest::geoip_lookup(Callback<> cb) {
    // This is to ensure that when calling multiple times geoip_lookup we
    // always reset the probe_ip, probe_asn and probe_cc values.
    probe_ip = "127.0.0.1";
    probe_asn = "AS0";
    probe_cc = "ZZ";
    ip_lookup(
        [=](Error err, std::string ip) {
            if (err) {
                logger->warn("ip_lookup() failed: error code: %d", err.code);
            } else {
                logger->info("probe ip: %s", ip.c_str());
                if (options.get("save_real_probe_ip", false)) {
                    logger->debug("saving user's real ip on user's request");
                    probe_ip = ip;
                }
                std::string country_p =
                    options.get("geoip_country_path", std::string{});
                std::string asn_p =
                    options.get("geoip_asn_path", std::string{});
                if (country_p == "" or asn_p == "") {
                    logger->warn("geoip files not configured; skipping");
                } else {
                    ErrorOr<nlohmann::json> res = geoip(ip, country_p, asn_p);
                    if (!!res) {
                        logger->debug("GeoIP result: %s", res->dump().c_str());
                        // Since `geoip()` sets defaults before querying, the
                        // following accesses of json should not fail unless for
                        // programmer error after refactoring. In that case,
                        // better to let the exception unwind than just print
                        // a warning, because the former is easier to notice
                        // and therefore fix during development
                        if (options.get("save_real_probe_asn", true)) {
                            probe_asn = (*res)["asn"];
                        }
                        logger->info("probe_asn: %s", probe_asn.c_str());
                        if (options.get("save_real_probe_cc", true)) {
                            probe_cc = (*res)["country_code"];
                        }
                        logger->info("probe_cc: %s", probe_cc.c_str());
                    }
                }
            }
            cb();
        },
        options, reactor, logger);
}

void OoniTest::open_report(Callback<Error> callback) {
    report.test_name = test_name;
    report.test_version = test_version;
    report.test_start_time = test_start_time;

    report.options = options;

    report.probe_ip = probe_ip;
    report.probe_cc = probe_cc;
    report.probe_asn = probe_asn;

    if (output_filepath == "") {
        output_filepath = generate_output_filepath();
    }
    if (options.find("no_file_report") == options.end()) {
        report.add_reporter(FileReporter::make(output_filepath));
    }
    if (options.find("no_collector") == options.end()) {
        report.add_reporter(OoniReporter::make(*this));
    }
    report.open(callback);
}

std::string OoniTest::generate_output_filepath() {
    int idx = 0;
    std::stringstream filename;
    while (true) {
        filename.str("");
        filename.clear();

        char timestamp[100];
        strftime(timestamp, sizeof(timestamp), "%FT%H%M%SZ", &test_start_time);
        filename << "report-" << test_name << "-";
        filename << timestamp << "-" << idx << ".json";

        std::ifstream output_file(filename.str().c_str());
        // If a file called this way already exists we increment the counter
        if (output_file.good()) {
            output_file.close();
            idx++;
            continue;
        }
        break;
    }
    return filename.str();
}

void OoniTest::begin(Callback<Error> cb) {
    if (begin_cb) {
        begin_cb();
    }
    mk::utc_time_now(&test_start_time);
    geoip_lookup([=]() {
        resolver_lookup([=](Error error, std::string resolver_ip_) {
            if (!error) {
                resolver_ip = resolver_ip_;
            } else {
                logger->debug("failed to lookup resolver ip");
            }
            open_report([=](Error error) {
                if (error) {
                    cb(error);
                    return;
                }
                size_t num_entries = 0;
                if (needs_input) {
                    if (input_filepath == "") {
                        logger->warn("an input file is required");
                        cb(MissingRequiredInputFileError());
                        return;
                    }
                    input_generator.reset(new std::ifstream(input_filepath));
                    if (!input_generator->good()) {
                        logger->warn("cannot read input file");
                        cb(CannotOpenInputFileError());
                        return;
                    }

                    // Count the number of entries
                    std::string next_input;
                    while ((std::getline(*input_generator, next_input))) {
                        num_entries += 1;
                    }
                    if (!input_generator->eof()) {
                        logger->warn("cannot read input file");
                        cb(FileIoError());
                        return;
                    }
                    // See http://stackoverflow.com/questions/5750485
                    //  and http://stackoverflow.com/questions/28331017
                    input_generator->clear();
                    input_generator->seekg(0);

                } else {
                    input_generator.reset(new std::istringstream("\n"));
                    num_entries = 1;
                }


                // Run `parallelism` measurements in parallel
                Var<size_t> current_entry(new size_t(0));
                mk::parallel(
                    mk::fmap<size_t, Continuation<Error>>(
                        mk::range<size_t>(options.get("parallelism", 3)),
                        [=](size_t thread_id) {
                            return [=](Callback<Error> cb) {
                                run_next_measurement(thread_id, cb, num_entries,
                                                     current_entry);
                            };
                        }),
                    cb);

            });
        }, options, reactor, logger);
    });
}

void OoniTest::end(Callback<Error> cb) {
    if (end_cb) {
        end_cb();
    }
    report.close(cb);
}

} // namespace ooni
} // namespace mk