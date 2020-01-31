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

// TODO: Readable stream does not seem to be reusable (one-time use)
// Need to figure out a way to use it, or switch to something else. Duplex Stream? EventEmitter?
// Note: Need to make sure all random numbers are created on addon_side synchronously before returning.
// this way multiple calls can be made, but they sequence won't get out of order (callback part is async)
// Alternative is only one call is allowed at a time to the class (to maintain order)
// 
// ex: generate, generate, generate
// - should always get back the same random numbers (1,2,3,4,5,etc...) even though they won't receive them at same time/order as before
//
// usecase:
/*
    let random_numbers = [];
    addon.GenerateSequenceStream(0, 100, 100); <<-- This should probably return a Readable object (one-time use)
    for await(const chunk : addon.GenerateSequenceSstream(0, 100, 100))
        random_numbers.push_back chunk
    
    ...

    Fill them again or fill a different one calling GenerateSequence again
    
*/

// RandSeed to inherit from Readable
let randSeedAddon = new RandSeed(Readable);
randSeedAddon.pipe(w);
randSeedAddon.GenerateSequenceStream(0, 100, 100);// Generate buffer async

console.log('start interval')
setInterval(() => {
    console.log('hello from JS')
}, 500)