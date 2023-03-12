const decoder = new TextDecoder();
const encoder = new TextEncoder();

const HEADER_SIZE = 8;
const header = encoder.encode("VPak 1.1");

function u8Equals(a: Uint8Array, b: Uint8Array) {
    if (a.length !== b.length) return false;

    for (let i = 0; i < a.length; i++) {
        if (a[i] !== b[i]) return false;
    }

    return true;
}

export function serialize(data: Record<string, Uint8Array>): Uint8Array {
    let size = HEADER_SIZE;
    const keys = [] as Uint8Array[];

    for (const key in data) {
        const encodedKey = encoder.encode(key);
        keys.push(encodedKey);
        size +=
            1 + // u8 for key length
            encodedKey.byteLength +
            4 + // u32 for data length
            data[key].byteLength;
    }

    const res = new Uint8Array(size);
    res.set(header, 0);

    let offset = HEADER_SIZE;
    let i = 0;
    for (const key in data) {
        // write u8 key length and key
        const encodedKey = keys[i++];
        res[offset++] = encodedKey.byteLength;
        res.set(encodedKey, offset);
        offset += encodedKey.byteLength;

        // write u32 data length big endian
        const dataLength = data[key].byteLength;
        res[offset++] = dataLength >> 24 & 0xff;
        res[offset++] = dataLength >> 16 & 0xff;
        res[offset++] = dataLength >> 8 & 0xff;
        res[offset++] = dataLength & 0xff;

        // write data
        res.set(data[key], offset);
        offset += dataLength;
    }

    return res;
}

export function deserialize(data: Uint8Array): Record<string, Uint8Array> {
    if (!u8Equals(data.subarray(0, HEADER_SIZE), header)) {
        throw new Error("Invalid header");
    }

    const res = {} as Record<string, Uint8Array>;

    let offset = HEADER_SIZE;
    while (offset < data.length) {
        const keyLength = data[offset++];
        const key = decoder.decode(data.subarray(offset, offset += keyLength));

        const dataLength =
            data[offset++] << 24
            | data[offset++] << 16
            | data[offset++] << 8
            | data[offset++];

        res[key] = data.subarray(offset, offset + dataLength);
        offset += dataLength;
    }

    return res;
}
