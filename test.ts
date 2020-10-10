import { NodeRand_mt19937 } from './src'
let rng = new NodeRand_mt19937();
console.log(rng.Generate(0, 10));

// let rng = new NodeRand(uint8_t, std_mt19937, std_uniform_int_distribution); // T, GENERATOR, DISTRIBUTION<T>

/**
 * 
  SetSeed(seed:number): void; generator.seed(seed);
  Generate(min?:number, max?:number): number; if arg0 setMin, if arg1 setMax
  GenerateSequenceStream(count:number, min?:number, max?:number): Readable; count, if arg1 setMin, if arg2 setMax 
 * 
 */

 // let a = rng.Generate() // [T.min(), T.max()]
 // a = rng.Generate(10) // [10, T.max()]
 // a = rng.Generate(10, 20) // [10, 20]
 // a = rng.Generate()

 // rng.SetSeed(10);
 // rng.Generate(min, max);
 // rng.GenerateSequenceStream(min, max, count);

 // let a = new std_uniform_int_distribution(uint8_t);
 // a.param(min? = , max? = T.max())  
 // std_uniform_int_distribution.generate(std_mt19937)
 // std_uniform_int_distribution.generateSequence(std_m19937, 1000) // generator, count

 // TODO: Test Generate vs GenerateSequence speed