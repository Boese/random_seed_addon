import { NodeRand_mt19937 as NodeRand } from '../src'
import { Readable, Writable } from 'stream'

import 'mocha'
import chai = require('chai')
import sinon = require('sinon')
import sinonChai = require('sinon-chai')
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
    ClearNumbers = () : void => { this.randNumbers = []; } 
}

describe('Random Seed', () => {

    it('Check multiple instances with same seed', () => {
        let a = new NodeRand();
        let b = new NodeRand();
        a.SetSeed(11);
        b.SetSeed(11);
        let a_rand = a.Generate(TEST_MIN, TEST_MAX);
        let b_rand = b.Generate(TEST_MIN, TEST_MAX);
        chai.expect(a_rand).to.be.equal(b_rand);
    })
    it('Check resetting seed on same instance', () => {
        // Call rand() twice
        let a = new NodeRand();
        a.SetSeed(TEST_SEED);

        let num1 = a.Generate(TEST_MIN, TEST_MAX);
        let num2 = a.Generate(TEST_MIN, TEST_MAX);

        // Reset seed
        a.SetSeed(TEST_SEED);

        // Call rand() twice again
        let num3 = a.Generate(TEST_MIN, TEST_MAX);
        let num4 = a.Generate(TEST_MIN, TEST_MAX);

        // Should equal
        chai.expect(num1).to.equal(num3);
        chai.expect(num2).to.equal(num4);
    })

    it('Check Generate 100x should match sequence of GenerateSequenceStream', async () => {

        const RangeToTest = 100;
        
        let a = new NodeRand();
        a.SetSeed(TEST_SEED);

        // Get 3 random numbers
        let nums: Number[] = [];
        for (let i = 0; i < RangeToTest; i++) {
            nums.push(a.Generate(TEST_MIN, TEST_MAX))
        }

        // Reset seed seed
        a.SetSeed(TEST_SEED);

        // Get 3 random numbers from rand sequence
        let w = new TestWriteableStream({});
        a.GenerateSequenceStream(TEST_MIN, TEST_MAX, RangeToTest).pipe(w);

        // Check (manual) Generate == GenerateSequenceStream
        let nums2: Number[] = await new Promise(resolve => w.on('finish', () => resolve(w.GetNumbers())));
        chai.expect(nums).eql(nums2);
    })

    it('Check 3 generate streams of size 1000, same seed, triggered in parallel, should be equal', async () => {

        const RangeToTest = 1000;

        let w1 = new TestWriteableStream({});
        let w2 = new TestWriteableStream({});
        let w3 = new TestWriteableStream({});

        let r1 = new NodeRand();
        let r2 = new NodeRand();
        let r3 = new NodeRand();

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

    it('Check GenerateSequenceStream multiple times off same instance, reset seed, run again, compare results, expect equal', async () => {
        const RangeToTest = 5;

        let w1 = new TestWriteableStream({});
        let r1 = new NodeRand();

        r1.SetSeed(TEST_SEED);

        let promiseWrapper = (r: any, w: TestWriteableStream) => {
            return new Promise<Number[]>(resolve => {
                let readableStream: Readable = r.GenerateSequenceStream(TEST_MIN, TEST_MAX, RangeToTest);
                readableStream.pipe(w, {end: false});
                readableStream.on('end', () => {
                    let nums = w.GetNumbers();
                    w.ClearNumbers();
                    resolve(nums)
                })
            })
        }

        let p1: Number[] = [];
        p1 = p1.concat(await promiseWrapper(r1, w1));
        p1 = p1.concat(await promiseWrapper(r1, w1));
        p1 = p1.concat(await promiseWrapper(r1, w1));

        r1.SetSeed(TEST_SEED)

        let p2: Number[] = [];
        p2 = p2.concat(await promiseWrapper(r1, w1));
        p2 = p2.concat(await promiseWrapper(r1, w1));
        p2 = p2.concat(await promiseWrapper(r1, w1));

        chai.expect(p1).eql(p2);
    })

    it('Check GenerateSequenceStream multiple times off same instance, reset seed, run in parallel, compare results, expect equal', async () => {
        const RangeToTest = 500;
        const loops = 10;

        let w1 = new TestWriteableStream({});
        let r1 = new NodeRand();

        r1.SetSeed(TEST_SEED);

        let promiseWrapper = (r: any, w: TestWriteableStream) => {
            return new Promise<Number[]>(resolve => {
                let readableStream: Readable = r.GenerateSequenceStream(TEST_MIN, TEST_MAX, RangeToTest);
                readableStream.pipe(w, {end: false});
                readableStream.on('end', () => {
                    let nums = w.GetNumbers();
                    w.ClearNumbers();
                    resolve(nums)
                })
            })
        }

        let p1: Number[] = [];
        for (let i = 0; i < loops; i++ ) {
            p1 = p1.concat(await promiseWrapper(r1, w1));
        }

        r1.SetSeed(TEST_SEED)

        let p2: Number[] = (await Promise.all([...Array(loops)].map(m => promiseWrapper(r1, w1)))).reduce((a,c) => a.concat(c))

        chai.expect(p1).eql(p2);
    })

    it('Check Number.MIN_SAFE_INTEGER <= number <= Number.MAX_SAFE_INTEGER', async () => {
        let r1 = new NodeRand();

        // Generate Max
        let num = r1.Generate(Number.MAX_SAFE_INTEGER, Number.MAX_SAFE_INTEGER);
        chai.expect(num).lte(Number.MAX_SAFE_INTEGER);

        // Generate Min
        num = r1.Generate(Number.MIN_SAFE_INTEGER, Number.MIN_SAFE_INTEGER);
        chai.expect(num).gte(-Number.MAX_SAFE_INTEGER);

        let w1 = new TestWriteableStream({});

        let promiseWrapper = (r: any, w: TestWriteableStream, min: Number, max: Number, count: Number) => {
            return new Promise<Number[]>(resolve => {
                let readableStream: Readable = r.GenerateSequenceStream(min, max, count);
                readableStream.pipe(w, {end: false});
                readableStream.on('end', () => {
                    let nums = w.GetNumbers();
                    w.ClearNumbers();
                    resolve(nums)
                })
            })
        }

        // GenerateSequenceStream Max
        let num2 = (await promiseWrapper(r1, w1, Number.MAX_SAFE_INTEGER, Number.MAX_SAFE_INTEGER, 1))[0];
        chai.expect(num).lte(0x1FFFFFFFFFFFFF);

        // GenerateSequenceStream Min
        num2 = (await promiseWrapper(r1, w1, Number.MIN_SAFE_INTEGER, Number.MIN_SAFE_INTEGER, 1))[0];
        chai.expect(num).gte(-0x1FFFFFFFFFFFFF);
    })

    it('Check different random numbers on multiple instances', async () => {
        const RangeToTest = 5;

        let w1 = new TestWriteableStream({});
        let r1 = new NodeRand();
        let r2 = new NodeRand();
        let r3 = new NodeRand();

        let promiseWrapper = (r: any, w: TestWriteableStream) => {
            return new Promise<Number[]>(resolve => {
                let readableStream: Readable = r.GenerateSequenceStream(TEST_MIN, TEST_MAX, RangeToTest);
                readableStream.pipe(w, {end: false});
                readableStream.on('end', () => {
                    let nums = w.GetNumbers();
                    w.ClearNumbers();
                    resolve(nums)
                })
            })
        }

        let p1: Number[] = await promiseWrapper(r1, w1);
        let p2: Number[] = await promiseWrapper(r2, w1);
        let p3: Number[] = await promiseWrapper(r3, w1);

        chai.expect(p1).not.eq(p2).not.eq(p3)
    })

    it('Check different random numbers when setting seed to different values', async () => {
        const RangeToTest = 5;

        let w1 = new TestWriteableStream({});
        let r1 = new NodeRand();

        r1.SetSeed(TEST_SEED);

        let promiseWrapper = (r: any, w: TestWriteableStream) => {
            return new Promise<Number[]>(resolve => {
                let readableStream: Readable = r.GenerateSequenceStream(TEST_MIN, TEST_MAX, RangeToTest);
                readableStream.pipe(w, {end: false});
                readableStream.on('end', () => {
                    let nums = w.GetNumbers();
                    w.ClearNumbers();
                    resolve(nums)
                })
            })
        }

        let p1: Number[] = await promiseWrapper(r1, w1);
        r1.SetSeed(TEST_SEED+1)
        let p2: Number[] = await promiseWrapper(r1, w1);

        chai.expect(p1).not.eq(p2);
    })
    
    // TODO: Add these tests in future when implemented (TDD style)
    // 1. Test everything here, but with different types, BigInt64, Int64 (MAX_JS_NUMBER), uint32/int32, uint16/int16, uint8/int8, double
    
})