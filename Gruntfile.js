module.exports = function(grunt) {

    grunt.initConfig({
      copy: {   
        release: {
            files: [
               { expand: true, src: 'src/build/Release/**/node_rand.node', dest: 'src/', flatten: true },
               { expand: true, src: 'src/build/Release/**/node_rand.node', dest: 'dist/', flatten: true }
            ]
        },
        debug: {
            files: [
               { expand: true, src: 'src/build/Debug/**/node_rand.node', dest: 'src/', flatten: true },
               { expand: true, src: 'src/build/Debug/**/node_rand.node', dest: 'dist/', flatten: true }
            ]
        }
      },

      build: {
        release: {},
        debug: {}
      },

      mochaTest: {
        test: {
          options: {
            reporter: 'spec',
            require: 'ts-node/register',
          },
          src: ['test/**/*.spec.ts']
        }
      }
    });
  
    grunt.loadNpmTasks('grunt-contrib-copy');
    grunt.loadNpmTasks('grunt-mocha-test');

    grunt.registerMultiTask('build', 'Execute node-gyp', function() {

      let done = this.async();

      let nodeGypCmd = 'node-gyp';
      let nodeGypArgs = ["configure", "&&", "node-gyp", "build", "-j", "4", `--${this.target}`]

      grunt.util.spawn({cmd: nodeGypCmd, args: nodeGypArgs, opts: {cwd: 'src'}}, function(error, result, code) {
        if (error) {
          grunt.fail.fatal(error);
        }

        grunt.log.ok("Finished Executing."); 
        return done();
      });

      grunt.log.debug(`Executing: ${nodeGypCmd}`, ...nodeGypArgs);
    });

  };