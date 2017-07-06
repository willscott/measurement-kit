// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "private/common/parallel.hpp"
#include "private/common/utils.hpp"
#include "private/ooni/captive_portal.hpp"
#include "private/ooni/constants.hpp"
#include "private/ooni/utils.hpp"

namespace mk {
namespace ooni {

using namespace mk::report;

auto status_code_correct = [=](Var<http::Response> response, std::string expected) {
    return response->status_code == std::stoi(expected); // this might be avoidable
};

auto body_correct = [=](Var<http::Response> response, std::string expected) {
    return response->body == expected;
};

// all but one of the current vendor tests are HTTP tests.
// these check either a status code or the body content.
typedef std::map<std::string,std::string> input_t;
void gen_http_inputs(Var<std::vector<input_t>> is, Var<Logger> logger) {
    logger->info("starting to gen");
    input_t i;
    i["name"] = "Microsoft HTTP";
    i["url"] = "http://www.msftncsi.com/ncsi.txt";
    i["body"] = "Microsoft NCSI";
    is->push_back(i);
    i.clear();
    i["name"] = "Apple HTTP 1";
    i["url"] = "http://captive.apple.com";
    i["body"] = "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>\n";
    i["ua"] = "CaptiveNetworkSupport/1.0 wispr";
    is->push_back(i);
    i.clear();
    i["name"] = "Apple HTTP 2";
    i["url"] = "http://captive.apple.com/hotspot_detect.html";
    i["body"] = "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>\n";
    i["ua"] = "CaptiveNetworkSupport/1.0 wispr";
    is->push_back(i);
    i.clear();
    i["name"] = "Android KitKat HTTP";
    i["url"] = "http://clients3.google.com/generate_204";
    i["status"] = "204"; //XXX: easier to just convert to an int later...
    is->push_back(i);
    i.clear();
    i["name"] = "Android Lollipop HTTP";
    i["url"] = "http://connectivitycheck.android.com/generate_204";
    i["status"] = "204";
    is->push_back(i);
    i.clear();
    i["name"] = "Android Marshmallow HTTP";
    i["url"] = "http://connectivitycheck.gstatic.com/generate_204";
    i["status"] = "204";
    is->push_back(i);
    i.clear();
    i["name"] = "Android Nougat HTTP 1";
    i["url"] = "https://www.google.com/generate_204";
    i["status"] = "204";
    i["ua"] = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36";
    is->push_back(i);
    i.clear();
    i["name"] = "Android Nougat HTTP 2";
    i["url"] = "http://www.google.com/gen_204";
    i["status"] = "204";
    i["ua"] = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36";
    is->push_back(i);
    i.clear();
    i["name"] = "Android Nougat HTTP 3";
    i["url"] = "http://play.googleapis.com/generate_204";
    i["status"] = "204";
    i["ua"] = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.82 Safari/537.36";
    is->push_back(i);
    i.clear();
}

void http_many(Var<Entry> entry,
               Callback<Error> all_done_cb,
               Var<Reactor> reactor,
               Var<Logger> logger) {

    auto http_cb = [=](const input_t& input, Callback<Error> done_cb) {
        return [=](Error err, Var<http::Response> response) {
            // result: true means unfiltered
            Entry result = { {"name", input.at("name")},
                             {"result", nullptr},
                             {"url", input.at("url")},
                             {"expected_body", nullptr},
                             {"actual_body", nullptr},
                             {"expected_status", nullptr},
                             {"actual_status", nullptr},
                             {"failure", nullptr} };

            if (input.count("body")) {
                result["expected_body"] = input.at("body");
            }
            if (input.count("status")) {
                result["expected_status"] = input.at("status");
            }

            if (!!err) {
                logger->info("err: %s", err.as_ooni_error().c_str());
                result["failure"] = err.as_ooni_error();
            } else if (!response) {
                logger->info("null response");
                result["failure"] = "null response"; // XXX correct string?
            } else {
                result["actual_body"] = response->body;
                result["actual_status"] = response->status_code;
                // all tests check status or body but never both or neither
                result["actual_body"] = response->body;
                bool unfiltered;
                if (input.count("body")) {
                    unfiltered = !!(input.at("body") == response->body);
                } else if (input.count("status")) {
                    unfiltered = !!(std::stoi(input.at("status")) == response->status_code);
                } else {
                    exit(1);
                }
                logger->info("%s: %d", input.at("name").c_str(), unfiltered);
                result["result"] = unfiltered;
            }
            (*entry)["vendor_http_tests"].push_back(result);
            done_cb(err);
        };
    };

    Var<std::vector<input_t>> inputs(new std::vector<input_t>);
    gen_http_inputs(inputs, logger);

    std::vector<Continuation<Error>> continuations;
    for (const auto& input : *inputs) {
        logger->info("setting up %s", input.at("name").c_str());
        Settings http_options; //XXX: specify timeout here
        http_options["http/url"] = input.at("url");
        std::string body;
        http::Headers headers;
        if (input.count("ua")) {
            headers = { { "User-Agent", input.at("ua") } };
        } else {
            headers = constants::COMMON_CLIENT_HEADERS;
        }

        continuations.push_back(
            [=](Callback<Error> done_cb) {
                templates::http_request(entry, http_options, headers, body,
                    http_cb(input, done_cb), reactor, logger);
            }
        );
    }
    mk::parallel(continuations, all_done_cb, 3);
}

// this is the only test that doesn't follow the pattern of those above.
// this hostname should always resolve to this IP.
void dns_msft_ncsi(Var<Entry> entry,
                   Callback<Error> done_cb,
                   Var<Reactor> reactor,
                   Var<Logger> logger) {
    Settings options;
    std::string hostname = "dns.msftncsi.com";
    std::string nameserver = "";
    templates::dns_query(
        entry, "A", "IN", hostname, nameserver,
        [=](Error err, Var<dns::Message> message) {
            Entry result = { {"name", "Microsoft DNS"},
                             {"result", nullptr},
                             {"hostname", "dns.msftncsi.com"},
                             {"expected_ip", "131.107.255.255"},
                             {"failure", nullptr} };
            if (!!err) {
                logger->info("dns_query err: %s", err.as_ooni_error().c_str());
                result["failure"] = err.as_ooni_error();
                done_cb(err);
            } else {
                for (const auto& a : message->answers) {
                    result["actual_ips"].push_back(a.ipv4);
                }
                if (message->answers.size() != 1) {
                    logger->info("maybe captive portal");
                    result["result"] = false;
                } else if (message->answers[0].ipv4 != "131.107.255.255") {
                    logger->info("probably captive portal");
                    result["result"] = false;
                } else {
                    logger->info("no captive portal");
                    result["result"] = true;
                }
                (*entry)["vendor_dns_tests"].push_back(result);
                done_cb(NoError());
            }
        }, options, reactor, logger);
}

void captive_portal(std::string input, Settings options,
                    Callback<Var<Entry>> callback, Var<Reactor> reactor,
                    Var<Logger> logger) {
    Var<Entry> entry(new Entry);

    logger->info("starting http_many");
    http_many(entry, [=](Error err){
        if (!!err) {
            logger->info("http_many error");
        }
        logger->info("starting dns_msft_ncsi");
        dns_msft_ncsi(entry, [=](Error err){
            if (!!err) {
                logger->info("dns_msft_ncsi error");
            }
            callback(entry);
        }, reactor, logger);
    }, reactor, logger);


}

} // namespace ooni
} // namespace mk
