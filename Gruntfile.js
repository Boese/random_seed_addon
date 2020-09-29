const childprocess = require('child_process');

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
        options: {
          j: '-j=threads',
          log_verbose: 'verbose mode'
        },
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

      const nodeCWD = 'src';
      const nodeGypCmd = 'node-gyp';
      const nodeGypConfigureArgs = ["configure", `--${this.target}`]
      const nodeGypBuildArgs = ["build", "-j", `${grunt.option('j') || 1}`, `--${this.target}`, `${grunt.option('j') ? '-v' : ''}`]

      // Configure
      grunt.log.writeln(`Executing: ${nodeGypCmd}`, ...nodeGypConfigureArgs);      
      ({pid, output, stdout, stderr, status, signal, error} = childprocess.spawnSync(nodeGypCmd, nodeGypConfigureArgs, {cwd: nodeCWD, shell: true, stdio: 'inherit', windowsHide: true}));
      if (error) {
        grunt.fail.fatal(error);
      }

      // Build
      grunt.log.writeln(`Executing: ${nodeGypCmd}`, ...nodeGypBuildArgs);      
      ({pid, output, stdout, stderr, status, signal, error} = childprocess.spawnSync(nodeGypCmd, nodeGypBuildArgs, {cwd: nodeCWD, shell: true, stdio: 'inherit', windowsHide: true}));
      if (error) {
        grunt.fail.fatal(error);
      }

      grunt.log.ok("Finished Executing."); 
      return done();

      
    });

  };