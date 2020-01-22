let RandSeed = require("./build/Release/random_seed.node").RandSeed
import { Writable, Readable } from 'stream'
import { EventEmitter } from 'events'
import { inherits } from 'util'


class RandSeed2 {
    constructor() {
        Readable.call(this, {})
    }
}
// Addon to inherit Writeable
inherits(RandSeed2, Readable )

export class RandomGenerator {
    private _r:any;
    

    constructor() {

        let w = new Writable({
            write(chunk, encoding, callback) {
                console.log(chunk.toString());
                callback();
            }
        })
        //let r = new Readable({})

        

        // r.push("hello");
        // r.push("world");
        // r.push(null);
        // console.log('data pushed');
        // r.pipe(w);

        this._r = new RandSeed2();
        this._r.push("hello");
        this._r.pipe(w);
        this._r.SetSeed(10);
        this._r.GenerateSequenceStream(0, 10, 10);


        // console.log(this._r.Generate(0,10));
        // console.log(this._r.Generate(0,10));
        // console.log(this._r.Generate(0,10));

        // let a = new RandSeed();
        // a.SetSeed(10);
        // console.log(a.Generate(0,10));
        // console.log(a.Generate(0,10));
        // console.log(a.Generate(0,10));
        
    }

    seed(seed?: number): void {
        this._r.seed(seed);
    }

    generate(min: number = 0, max: number = Number.MAX_SAFE_INTEGER) : number {
        return this._r.generate(min, max);
    }
    
    generateSequenceAsync(min: number = 0, max: number = Number.MAX_SAFE_INTEGER, size: number) : Promise<number[]> {
        return new Promise((resolve, reject) => {

            let random_sequence:number[] = [];
            
            let currentByteCount = 0;
            let num = new Uint8Array(4); // rotating buffer
        })
    }

    // generateSequenceStream = (min: number = 0, max: number = Number.MAX_SAFE_INTEGER, size: number) : Writable => {
    //     console.log('generate')
    //     let addon_stream = new Writable();
        
    //     let read_stream = new Readable({
    //         read(size:number) {
    //             console.log('read', size);
    //         }
    //     });
    //     read_stream.on('data', (chunk: any) => {
    //         console.log('data', chunk);
    //     })
    //     read_stream.on('close', () => {
    //         console.log('done');
    //     })
    //     read_stream.pipe(addon_stream);

    //     this._r.sequence(min, max, size);

    //     return addon_stream;
    // }
}

// let a = new RandomGenerator();
// console.log('new class');
