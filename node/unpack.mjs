import { readFileSync, writeFileSync } from "fs";
import { HEADER } from "./shared.mjs";

if (process.argv.length !== 3) {
    console.log(`Usage: ${process.argv[0]} ${process.argv[1]} BUNDLE`);
    process.exit();
}

const content = readFileSync(process.argv[2]);
const size = content.byteLength;
let offset = 0;

function readU32() {
    const read = content.readUint32LE(offset);
    offset += 4;
    return read;
}

function readU8() {
    const read = content.readUint8(offset);
    offset += 1;
    return read;
}

function read(size) {
    const buf = content.subarray(offset, offset + size);
    offset += size;
    return buf;
}

if (!HEADER.equals(read(HEADER.byteLength))) {
    console.error("Invalid Header. Is this even a VPak file?");
    process.exit(1);
}

while (offset < size) {
    const nameSize = readU8();
    const name = read(nameSize);
    console.log(`File: ${name}`);

    const size = readU32();
    console.log("Size:", size);

    const bytes = read(size);

    const outName = name.toString() + ".unpacked";
    writeFileSync(outName, bytes);
    console.log("Wrote to", outName);
}
