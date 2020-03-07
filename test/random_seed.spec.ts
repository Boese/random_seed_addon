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

    it('3 generate streams of size 1000, same seed, triggered in parallel, should be equal', async () => {

        const RangeToTest = 1000;

        let w1 = new TestWriteableStream({});
        let w2 = new TestWriteableStream({});
        let w3 = new TestWriteableStream({});

        let r1 = new RandSeed();
        let r2 = new RandSeed();
        let r3 = new RandSeed();

        r1.SetSeed(TEST_SEED);
        r2.SetSeed(TEST_SEED);
        r3.SetSeed(TEST_SEED);

        let promiseWrapper = (r: any, w: TestWriteableStream) => {
            return new Promise<Number[]>(resolve => {
                r.GenerateSequenceStream(TEST_MIN, TEST_MAX, RangeToTest).pipe(w);
                w.on('finish', () => resolve(w.GetNumbers()));
            })
        }

        let p1 = promiseWrapper(r1, w1);
        let p2 = promiseWrapper(r2, w2);
        let p3 = promiseWrapper(r3, w3);

        let results: Number[][] = await Promise.all([p1, p2, p3]);
        chai.expect(results[0]).eql(results[1]).eql(results[2]);
    })

    // TODO: Add tests for these cases
    // 1. Call GenerateSequenceStream multiple times off same instance, reset seed, run again, compare results, expect equal
    // 2. Same as 1, but run them in parallel (in hopes they will run/finish at different times)
    // 3. Test Max/Min
    
    // TODO: Add these tests in future when implemented (TDD style)
    // 1. Test everything here, but with different types, BigInt64, Int64 (MAX_JS_NUMBER), uint32/int32, uint16/int16, uint8/int8, double
    
})