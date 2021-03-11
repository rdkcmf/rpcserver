# JSON RPC 2.0 documentation generator

## Preface

Provided tools are intended to generate API documentation for APIs implemented using `onemw-src/rdk/rpcserver` library.

Provided tools are for:

* Generating Markdown documentation from JSON schema specifications using slightly modified https://github.com/rdkcentral/Thunder/tree/master/Tools/JsonGenerator tool
* Uploading prepared Markdown to Confluence

An example on how to use these tools can be found at `onemw-src/av/sessionmgr/api/wsrpc`.

## Prerequisites

Provided tools require `Python 3`.

For documentation generation using `GenerateDocs.sh` you'll need `jsonref` library:

```sh
pip3 install jsonref
```

For uploading to Confluence you'll need `requests` and `atlassian-python-api` libraries:

```sh
pip3 install requests
pip3 install atlassian-python-api
```

## Generating documentation

Input format is JSON schema based on `https://github.com/rdkcentral/Thunder/blob/master/Tools/JsonGenerator/schemas/interface.schema.json`. Sample schema files can be found at `onemw-src/av/sessionmgr/api/wsrpc`.

Usage example (assuming current directory is `onemw-src/av/sessionmgr/api/wsrpc`):

```sh
../../../../rdk/rpcserver/docgen/GenerateDocs.sh
```

`GenerateDocs.sh` script will generate:

* Markdown documentation in `doc` directory that can be commited and used e.g. on Github directly
* Markdown documentation in `confluence` directory that is in special format usable when uploading to Confluence page

`GenerateDocs.sh` will read all JSON schema files in current directory and generate Mardown file for each input schema file in `doc` and `confluence` directories.

When `GenerateDocs.sh` is invoked it clones https://github.com/rdkcentral/Thunder repo to temp directory and applies patch to documentation generator. Temp directory is always at `onemw-src/rdk/rpcserver/docgen` regardless of current working directory. After first run `GenerateDocs.sh` uses 'cached' generator tool. To clean temp directory (e.g. when documentation generator is updated) you can run:

```sh
../../../../rdk/rpcserver/docgen/GenerateDocs.sh reset
```

## Uploading to Confluence

When Markdown files in `confluence` directory are generated they can be uploaded to specific Confluence using `ConfluenceUpload.py` tool.

Example (assuming current directory is `onemw-src/av/sessionmgr/api/wsrpc`):

```sh
../../../../rdk/rpcserver/docgen/ConfluenceUpload.py --markdown confluence/MyAPI.md --url https://wikiprojects.upc.biz --user user.name --password 12345678 --space CTOM --page "My API Page"
```

If needed generated MD files can be uploaded to Confluence manually, just choose `Insert > Markup` and select `Markdown` from drop-down menu.
