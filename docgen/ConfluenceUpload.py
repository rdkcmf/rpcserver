#!/usr/bin/env python3
##############################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2021 Liberty Global Service BV
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##############################################################################

import os
import sys
import argparse
import json
import requests
from requests.auth import HTTPBasicAuth
from atlassian import Confluence
from urllib.parse import urljoin

if __name__ == "__main__":
    argparser = argparse.ArgumentParser(
        description='Upload prepared MD to Confluence',
        formatter_class=argparse.RawTextHelpFormatter)
    argparser.add_argument(
        "--markdown",
        dest="markdown",
        metavar="MARKDOWN",
        action="store",
        type=str,
        required=True,
        help="Markdown file path"
    )
    argparser.add_argument(
        "--url",
        dest="url",
        metavar="URL",
        action="store",
        type=str,
        required=True,
        help="Confluence URL"
    )
    argparser.add_argument(
        "--user",
        dest="user",
        metavar="USER",
        action="store",
        type=str,
        required=True,
        help="Confluence user name"
    )
    argparser.add_argument(
        "--password",
        dest="password",
        metavar="PASSWORD",
        action="store",
        type=str,
        required=True,
        help="Confluence user password"
    )
    argparser.add_argument(
        "--space",
        dest="space",
        metavar="SPACE",
        action="store",
        type=str,
        required=True,
        help="Space key"
    )
    argparser.add_argument(
        "--page",
        dest="page",
        metavar="PAGE",
        action="store",
        type=str,
        required=True,
        help="Page name"
    )

    args = argparser.parse_args(sys.argv[1:])

    if args.markdown:
        args.markdown = os.path.abspath(os.path.normpath(args.markdown))

    confluence = Confluence(
        url=args.url,
        username=args.user,
        password=args.password)

    f = open(args.markdown, "r")
    md = f.read()
    f.close()

    req = {
        "wiki": md
    }

    converter_url = urljoin(args.url, "/rest/tinymce/1/markdownxhtmlconverter")

    resp = requests.post(converter_url,
                         json=req, auth=HTTPBasicAuth(args.user, args.password))

    if resp.status_code != 200:
        print("Can't convert MD to Confluence markup using " + converter_url)
        exit(1)

    # Add TOC to converted markup
    contents = "<p><ac:structured-macro ac:name=\"toc\"><ac:parameter ac:name=\"maxLevel\">3</ac:parameter></ac:structured-macro></p>" + resp.text
    resp.close()

    page_id = confluence.get_page_id(args.space, args.page)
    confluence.update_page(page_id, args.page, contents)

    confluence.close()
