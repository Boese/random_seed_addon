import { Writable, Readable } from "stream";

//const random_seed_module = `./build/Release/random_seed.node`
const random_seed_module = './random_seed.node';

export class RandomGenerator {
    private _r:any;
    constructor() {

        // NOTE: Required to reload module multiple times
        delete require.cache[require.resolve(random_seed_module)];
        this._r = require(random_seed_module);
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

    generateSequenceStream = (min: number = 0, max: number = Number.MAX_SAFE_INTEGER, size: number) : Writable => {
        console.log('generate')
        let addon_stream = new Writable();
        
        let read_stream = new Readable({
            read(size:number) {
                console.log('read', size);
            }
        });
        read_stream.on('data', (chunk: any) => {
            console.log('data', chunk);
        })
        read_stream.on('close', () => {
            console.log('done');
        })
        read_stream.pipe(addon_stream);

        this._r.sequence(min, max, size);

        return addon_stream;
    }

    // sequence(min: number, max: number, size: number): Array<Number> {
    //     let result: Array<Number> = [];
    //     let sequence_buffer: ArrayBuffer = this._r.sequence(min, max, size);
    //     let sequence_buffer_dataview:DataView = new DataView(sequence_buffer);

    //     for (let i = 0; i < size*4; i+=4) {
    //         result.push(sequence_buffer_dataview.getInt32(i, true));
    //     }

    //     return result;
    // }
}

console.log('test')
let test = new RandomGenerator();
test.generateSequenceStream(0, 10, 1);