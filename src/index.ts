let RandSeed = require("./build/Release/random_seed.node").RandSeed
import { Writable, Readable } from 'stream'

let i = 0;
let w = new Writable({
    write: (chunk: any, encoding: any, callback: any) => {
        let d = new DataView(chunk.buffer)
        let n = d.getInt32(15996, true)
        console.log(i++, n);
        callback();
    }
});

// TODO: Readable stream does not seem to be reusable (one-time use) DONE
// Need to make sure class pushes same order every time. One idea below
/*
    Upon create of addon -> generate 10000 random numbers in a buffer.
    Each time generateSequence is called use 1 random number in buffer to create new std::random_device and use that inside instance of readable
    Each time generate is called just return the random number from global buffer
    This should be faster, use less memory, but more complex ie rng per call, but destroyed upon exit
*/

// Other idea below
/*
    Every time generateSequence is called, generate all the numbers needed synchronously. Then asynchronously start returning numbers
    This will be slower but simpler ie only one std::rng per class

*/
// 
// ex: async generate, async generate, async generate
// - should always get back the same random numbers (1,2,3,4,5,etc...) even though they might not receive them at same time/order as before
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
let randSeedAddon = new RandSeed();
randSeedAddon.GenerateSequenceStream(Readable).pipe(w, {end: false});
randSeedAddon.GenerateSequenceStream(Readable).pipe(w, {end: false});
randSeedAddon.GenerateSequenceStream(Readable).pipe(w, {end: false});
randSeedAddon.GenerateSequenceStream(Readable).pipe(w, {end: false});
randSeedAddon.GenerateSequenceStream(Readable).pipe(w, {end: false});

console.log('start interval')
setInterval(() => {
    console.log('hello from JS')
}, 500)