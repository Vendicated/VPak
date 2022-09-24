# VPak

Simple tool that packs multiple files into one.

No compression. File names and content are the only info stored.
Also no error handling :trollface:

## Should I use this?
No.

## Usage

#### Pack

node pack.mjs file1 file2 ... outfile

#### Unpack

node unpack.mjs bundlefile

## Limitations

Filename must be <= 256 Bytes

Size of individual files must be <= 4GB
