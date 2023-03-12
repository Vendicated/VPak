import { createWriteStream, openSync, createReadStream, fstatSync, closeSync, existsSync } from "fs";
import { basename } from "path";
import { HEADER } from "./shared.mjs";

if (process.argv.length < 4) {
    console.log(`Usage: ${process.argv[0]} ${process.argv[1]} [...FILES] OUTFILE`);
    process.exit();
}

const outfile = process.argv.pop();
if (existsSync(outfile)) {
    console.warn(`Outfile ${outfile} exists. Not overwriting.`);
    process.exit();
}

const u8buf = Buffer.allocUnsafe(1);
const u32buf = Buffer.allocUnsafe(4);

const stream = createWriteStream(outfile, {
    encoding: "binary",
});

function writeU32(u32) {
    u32buf.writeUint32LE(u32);
    stream.write(u32buf);
}

function writeU8(u8) {
    u8buf.writeUint8(u8);
    stream.write(u8buf);
}

stream.write(HEADER);

for (const file of process.argv.slice(2)) {
    const fd = openSync(file, "r");
    const stats = fstatSync(fd);

    const fileNameBytes = Buffer.from(basename(file));
    writeU8(fileNameBytes.byteLength);
    stream.write(fileNameBytes);

    writeU32(stats.size);
    const rs = createReadStream(file, "binary");
    const p = new Promise(res =>
        rs.once("end", res)
    );
    rs.pipe(stream, { end: false });
    await p;
    closeSync(fd);
}
