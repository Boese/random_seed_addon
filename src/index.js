"use strict";
exports.__esModule = true;
var stream_1 = require("stream");
var RandomGenerator = /** @class */ (function () {
    function RandomGenerator() {
        // NOTE: Required to reload module multiple times
        delete require.cache[require.resolve("../build/Release/random_seed.node")];
        this._r = require("../build/Release/random_seed.node");
    }
    RandomGenerator.prototype.seed = function (seed) {
        this._r.seed(seed);
    };
    RandomGenerator.prototype.generate = function (min, max) {
        if (min === void 0) { min = 0; }
        if (max === void 0) { max = Number.MAX_SAFE_INTEGER; }
        return this._r.generate(min, max);
    };
    RandomGenerator.prototype.generateSequenceAsync = function (min, max, size) {
        if (min === void 0) { min = 0; }
        if (max === void 0) { max = Number.MAX_SAFE_INTEGER; }
        return new Promise(function (resolve, reject) {
            var random_sequence = [];
            var currentByteCount = 0;
            var num = new Uint8Array(4); // rotating buffer
        });
    };
    RandomGenerator.prototype.generateSequenceStream = function (min, max, size) {
        if (min === void 0) { min = 0; }
        if (max === void 0) { max = Number.MAX_SAFE_INTEGER; }
        console.log('generate');
        var addon_stream = new stream_1.Writable();
        var read_stream = new stream_1.Readable();
        read_stream.on('data', function (chunk) {
            console.log(chunk);
        });
        read_stream.on('close', function () {
            console.log('done');
        });
        read_stream.pipe(addon_stream);
        this._r.sequence(min, max, size, addon_stream.write.bind(addon_stream));
        return addon_stream;
    };
    return RandomGenerator;
}());
exports.RandomGenerator = RandomGenerator;
console.log('test');
var test = new RandomGenerator();
test.generateSequenceStream(0, 10, 1);
