// Type definitions for [NodeRand] [1.0.0]
// Project: [node_rand]
// Definitions by: [Chris Boese] <[https://github.com/Boese/node-rand]>

import { Readable } from 'stream'

declare class NodeRand {
  constructor();

  SetSeed(seed:number): void;
  Generate(min:number, max:number): number;
  GenerateSequenceStream(min:number, max:number, count:number): Readable;
}

