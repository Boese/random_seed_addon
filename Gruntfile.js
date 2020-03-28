module.exports = function(grunt) {

    // Project configuration.
    grunt.initConfig({
      copy: {   
        Release: {
            files: [
               { expand: true, src: 'src/build/Release/**/node_rand.node', dest: 'src/', flatten: true },
               { expand: true, src: 'src/build/Release/**/node_rand.node', dest: 'dist/', flatten: true }
            ]
        },
        Debug: {
            files: [
               { expand: true, src: 'src/build/Debug/**/node_rand.node', dest: 'src/', flatten: true },
               { expand: true, src: 'src/build/Debug/**/node_rand.node', dest: 'dist/', flatten: true }
            ]
        }
      }
    });
  
    grunt.loadNpmTasks('grunt-contrib-copy');
  
    grunt.registerTask('default', 'Copying release build to src/dest folders', ['copy:Release']);
    grunt.registerTask('copy-debug', 'Copying debug build to src/dest folders', ['copy:Debug']);
  
  };