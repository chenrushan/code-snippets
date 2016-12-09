#!/usr/bin/env python
# encoding: utf-8

import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Help iSCSI deployment.')
    # choices 就是这个选项只能取 choices 数组中给出的值
    parser.add_argument('-s', '--service', choices=['target', 'initiator'],
                        default=None, required=True, help='type of service deployment')
    parser.add_argument('--test', action='store_true', default=False,
                        help='Test the code against a local mkcloud installation')
    args = parser.parse_args()

    print args.service
    if args.test:
        print "test is true"

