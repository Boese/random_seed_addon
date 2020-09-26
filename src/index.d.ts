// Type definitions for [NodeRand] [1.0.0]
// Project: [node_rand]
// Definitions by: [Chris Boese] <[https://github.com/Boese/node-rand]>

import { Readable } from 'stream'

export class NodeRand_mt19937 {
  constructor();
  SetSeed(seed:number): void;
  Generate(min:number, max:number): number;
  GenerateSequenceStream(min:number, max:number, count:number): Readable;
}

export class NodeRand_mt19937_64 {
  constructor();
  SetSeed(seed:number): void;
  Generate(min:number, max:number): number;
  GenerateSequenceStream(min:number, max:number, count:number): Readable;
}