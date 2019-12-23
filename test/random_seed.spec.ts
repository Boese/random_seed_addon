import { RandomGenerator } from '../../src/random-seed'

import 'mocha'
import chai from 'chai'
import sinon from 'sinon'
import sinonChai from 'sinon-chai'
chai.use(sinonChai)

const TEST_SEED=10;
const TEST_MIN=0;
const TEST_MAX=1000;

describe('Random Seed', () => {

    let rand_seed: RandomGenerator;
    beforeEach(() => {
        rand_seed = new RandomGenerator();
        rand_seed.seed(TEST_SEED);
    })
    it('Check multiple instances', () => {
        let a = new RandomGenerator();
        let b = new RandomGenerator();
        a.seed(11);
        b.seed(11);
        let a_rand = a.generate(TEST_MIN, TEST_MAX);
        let b_rand = b.generate(TEST_MIN, TEST_MAX);
        chai.expect(a_rand).to.be.equal(b_rand);
    })
    it('Check resetting seed ', () => {
        // Call rand() twice
        let seed1 = rand_seed.generate(TEST_MIN, TEST_MAX);
        let seed2 = rand_seed.generate(TEST_MIN, TEST_MAX);

        // Reset seed
        rand_seed.seed(TEST_SEED);

        // Call rand() twice again
        let seed3 = rand_seed.generate(TEST_MIN, TEST_MAX);
        let seed4 = rand_seed.generate(TEST_MIN, TEST_MAX);

        // Should equal
        chai.expect(seed1).to.equal(seed3);
        chai.expect(seed2).to.equal(seed4);
    })

    it('should return correct sequence for 3 elements', () => {

        // Get 3 random numbers
        let a = [rand_seed.generate(TEST_MIN, TEST_MAX), rand_seed.generate(TEST_MIN, TEST_MAX), rand_seed.generate(TEST_MIN, TEST_MAX)];

        // Reset seed seed
        rand_seed.seed(TEST_SEED);

        // Get 3 random numbers from rand sequence
        let b = rand_seed.sequence(TEST_MIN, TEST_MAX, 3);

        // Check a,b,c = rand sequence
        chai.expect(a).eql(b);
    })
    
})