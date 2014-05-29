#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

import sys

sys.path.insert(0, "/usr/local/share/libneubot")

from libneubot import LIBNEUBOT

def main():
    poller = LIBNEUBOT.IghtPoller_construct()

    #
    # The following call should fail because we configured the function
    # to accept an instance of PollerBase only.
    #
    LIBNEUBOT.IghtEchoServer_construct(poller, 0, "127.0.0.1", "12345")
    LIBNEUBOT.IghtPoller_loop(poller)

if __name__ == "__main__":
    main()
