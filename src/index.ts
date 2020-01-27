let RandSeed = require("./build/Release/random_seed.node").RandSeed
import { Writable, Readable } from 'stream'

let w = new Writable({
    write(chunk: Uint8Array, encoding, callback) {
        let d = new DataView(chunk.buffer)
        let n = d.getInt32(15996, true)
        console.log(n);
        callback();
    }
})

// Pass AddonHelper function to call superCtor of Readable
let randSeedAddon = new RandSeed(Readable);
randSeedAddon.GenerateSequenceStream(0, 100, 100); // Generate buffer async
randSeedAddon.pipe(w);

// TODO: Create typings for this addon
//export class RandSeed;