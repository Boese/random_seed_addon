let RandSeed = require("./build/Release/random_seed.node").RandSeed
import { Writable, Readable } from 'stream'
import { inherits } from 'util'
inherits(RandSeed, Readable); // required

let w = new Writable({
    write(chunk: Uint8Array, encoding, callback) {
        let d = new DataView(chunk.buffer)
        let n = d.getInt32(15996, true)
        console.log(n);
        callback();
    }
})

// TODO: Can this be found from addon directly? If so Addon should call and not need cb supplied to ctor
// Return cb function that will call superCtor when invoked
let AddonCallSuperHelper = (superCtor: Function, ...ctor_args: any[]) => {
    return function() { superCtor.call(this, ...ctor_args); }
}
let RandSeedCallReadableSuper = AddonCallSuperHelper(Readable, {});

// Pass AddonHelper function to call superCtor of Readable
let randSeedAddon = new RandSeed(/*RandSeedCallReadableSuper*/);

// TODO: Replace TestFunc and pass directly to Ctor
randSeedAddon.TestFunc(RandSeedCallReadableSuper);
randSeedAddon.pipe(w);



//inherits(RandSeed, Readable);

// function RandSeedAddonHelper() {
//     if (!(this instanceof RandSeedAddonHelper))
//       return new (RandSeedAddonHelper as any)();
//     console.log('new RandSeedAddonHelper');
//     Readable.call(this, {encoding: 'utf8', objectMode: false});
//   }
//   inherits(RandSeedAddonHelper, Readable);


// console.log('new RandSeed()');
// let r: Readable = new RandSeed();
// //let r: Readable = RandSeedAddonHelper();

// (r as any).TestFunc(RandSeedAddonHelper);

// let w = new Writable({
//     write(chunk, encoding, callback) {
//         console.log(chunk.toString());
//         callback();
//     }
// })

// r.push("hello");
// r.push("world");
// r.push(null);
// console.log('data pushed');
// r.pipe(w);

// class RandSeedAddonHelper extends Readable {
//     constructor() {
//         console.log('helper called')
//         super({encoding: 'utf8'})
//     }
// }


// // Addon to extend Readable (w/Helper to call super() <- Not supported currently in node_api)
// //function RandSeedAddonHelper() { Readable.call(this) };
//inherits(RandSeed, Readable )

export class RandomGenerator {
    private _r:any;
    private TestFunc: Function;
    

    constructor() {

        this.TestFunc = () => {
            console.log("test func called");
        }

        function RandSeedAddonHelper() {
            if (!(this instanceof RandSeedAddonHelper))
              return new (RandSeedAddonHelper as any)();
            console.log('new RandSeedAddonHelper');
            Readable.call(this, {});
          }
          inherits(RandSeedAddonHelper, Readable);

        let w = new Writable({
            write(chunk, encoding, callback) {
                console.log(chunk.toString());
                callback();
            }
        })

        let r = RandSeedAddonHelper();

        

        r.push("hello");
        r.push("world");
        r.push(null);
        console.log('data pushed');
        r.pipe(w);

        this._r = new RandSeed();
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
