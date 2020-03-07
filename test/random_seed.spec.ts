let RandSeed = require("../dist/random_seed.node").RandSeed
import { Writable, Readable } from 'stream'
RandSeed.SetReadable(Readable) // required

import 'mocha'
import chai from 'chai'
import sinon from 'sinon'
import sinonChai from 'sinon-chai'
chai.use(sinonChai)

const TEST_SEED=10;
const TEST_MIN=-1000;
const TEST_MAX=1000;

class TestWriteableStream extends Writable {
    private randNumbers: Number[] = []
    constructor(options: any) {
        super(options);
    }
    _write(chunk: any, encoding: any, callback: any) {
        let d = new DataView(chunk.buffer)
        callback(null);
        
        for (let i = 0; i < d.byteLength; i+=8) {
            let num = d.getInt32(i, true) + d.getInt32(i+4, true);

            // TODO: WTH??? Why is this required???
            if (num < 0) {
                num += 1;
            }
            this.randNumbers.push(num);
        }
    }
    GetNumbers = () => this.randNumbers;
}

describe('Random Seed', () => {

    let rand_seed: any;
    it('check', () => {
        rand_seed = new RandSeed();
        chai.expect(10).to.eq(10);
    })

    beforeEach(() => {
        rand_seed = new RandSeed();
        rand_seed.SetSeed(TEST_SEED);
    })
    it('Check multiple instances', () => {
        let a = new RandSeed();
        let b = new RandSeed();
        a.SetSeed(11);
        b.SetSeed(11);
        let a_rand = a.Generate(TEST_MIN, TEST_MAX);
        let b_rand = b.Generate(TEST_MIN, TEST_MAX);
        chai.expect(a_rand).to.be.equal(b_rand);
    })
    it('Check resetting seed ', () => {
        // Call rand() twice
        let seed1 = rand_seed.Generate(TEST_MIN, TEST_MAX);
        let seed2 = rand_seed.Generate(TEST_MIN, TEST_MAX);

        // Reset seed
        rand_seed.SetSeed(TEST_SEED);

        // Call rand() twice again
        let seed3 = rand_seed.Generate(TEST_MIN, TEST_MAX);
        let seed4 = rand_seed.Generate(TEST_MIN, TEST_MAX);

        // Should equal
        chai.expect(seed1).to.equal(seed3);
        chai.expect(seed2).to.equal(seed4);
    })

    it('should return correct sequence for 100 elements', async () => {

        const RangeToTest = 100;

        // Get 3 random numbers
        let a: Number[] = [];
        for (let i = 0; i < RangeToTest; i++) {
            a.push(rand_seed.Generate(TEST_MIN, TEST_MAX))
        }

        // Reset seed seed
        rand_seed.SetSeed(TEST_SEED);

        // Get 3 random numbers from rand sequence
        let w = new TestWriteableStream({});
        rand_seed.GenerateSequenceStream(TEST_MIN, TEST_MAX, RangeToTest).pipe(w);

        // Check (manual) Generate == GenerateSequenceStream
        let b: Number[] = await new Promise(resolve => w.on('finish', () => resolve(w.GetNumbers())));
        chai.expect(a).eql(b);
    })
    
})