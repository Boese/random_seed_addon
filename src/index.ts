let RandSeed = require("./build/Release/random_seed.node").RandSeed
import { Writable, Readable } from 'stream'

class myWritable extends Writable {
    constructor() {
        super({});

        (this.write as any) = (chunk: any, encoding: any, callback: any) => {
            let d = new DataView(chunk.buffer)
            let n = d.getInt32(15996, true)
            console.log(n);
            if (callback)
                callback();
            return true;
        }

        this.on('error', (err) => {
            console.log(err)
        }) 
        this.on('finish', () => {
            console.log('All writes are now complete.');
          });
    }
}

// TODO: RandSeed needs to implement _push. Right now it just calls push(), but since it
// implements Readable _push is required. This will fix not_implemented error
class myReadable extends Readable {
    private readonly char: String = '0';
    private source: NodeJS.Timeout;
    private endNow: boolean = false;
    constructor() {
        super({})
    }

    private onData(chunk: string) {
        if (!this.endNow && !this.push(chunk)) {
            this.stop();
        }
    }

    private start(size: number) {
        this.stop();
        this.source = setInterval(() => {
            this.onData(this.char.repeat(size));
        }, 100)
    }

    private stop() {
        clearInterval(this.source);
    }

    _read(size: number) {
        // Start generating
        this.start(size);
    }

    end() {
        this.endNow = true;
        clearInterval(this.source);
        this.push(null);
        console.log('end')
    }
}

let r = new myReadable();
let w = new myWritable();

r.on('close', () => {
    console.log('close')
    r.unpipe(w);
    w.end();
})

r.pipe(w);
setTimeout(() => {
    r.end();
    r.destroy();
    w.end();
    w.destroy();
}, 500)

// RandSeed to inherit from Readable
let randSeedAddon = new RandSeed(Readable);
randSeedAddon.pipe(w);
randSeedAddon.GenerateSequenceStream(0, 100, 100, () => {
    randSeedAddon.pipe(w);
});// Generate buffer async

// setTimeout(() => {
//     for(let i = 0; i < 1000; i++) {
//         console.log('done')
//     }
// }, 100)

// TODO: Create typings for this addon
//export class RandSeed;