let RandSeed = require("./build/Release/random_seed.node").RandSeed
import { Writable, Readable } from 'stream'

let w = new Writable({
    write: (chunk: any, encoding: any, callback: any) => {
        let d = new DataView(chunk.buffer)
        let n = d.getInt32(15996, true)
        console.log(n);
        callback();
    }
});

// RandSeed to inherit from Readable
let randSeedAddon = new RandSeed(Readable);
randSeedAddon.pipe(w);
randSeedAddon.GenerateSequenceStream(0, 100, 100);// Generate buffer async

setInterval(() => {
    console.log('hello from JS')
}, 500)