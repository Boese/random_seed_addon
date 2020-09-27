// Type definitions for [NodeRand] [1.0.0]
// Project: [node_rand]
// Definitions by: [Chris Boese] <[https://github.com/Boese/node-rand]>

import { Readable } from 'stream'

// Not really an abstract class, just useful for definitions. This class is templated on the c++ random number generator type
// DON'T IMPORT
declare abstract class _NodeRand {
  SetSeed(seed:number): void;
  Generate(min:number, max:number): number;
  GenerateSequenceStream(min:number, max:number, count:number): Readable;
}

export class NodeRand_mt19937 extends _NodeRand {
  constructor();
}

export class NodeRand_mt19937_64 extends _NodeRand {
  constructor();
}