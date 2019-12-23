
export class RandomGenerator {
    private _r:any;
    constructor() {
        // NOTE: Required to reload module multiple times
        delete require.cache[require.resolve(`./build/Release/random_seed.node`)]
        this._r = require(`./build/Release/random_seed.node`);
    }

    seed(seed?: number): void {
        this._r.seed(seed);
    }

    generate(min: number, max: number) : number {
        return this._r.generate(min, max);
    } 

    sequence(min: number, max: number, size: number): Array<Number> {
        let result: Array<Number> = [];
        let sequence_buffer: ArrayBuffer = this._r.sequence(min, max, size);
        let sequence_buffer_dataview:DataView = new DataView(sequence_buffer);

        for (let i = 0; i < size*4; i+=4) {
            result.push(sequence_buffer_dataview.getInt32(i, true));
        }

        return result;
    }
}