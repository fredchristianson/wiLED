var env = null;
function nothing(){}
var log = {debug: nothing, warn:nothing,info:nothing,error:nothing};
if (typeof process === 'undefined') {
    // browser, so set to the correct env file.
    if (location.port == "6080") {
       // env = env_dev;
       const {ENV} =  await import('./browser/browser-env-dev.js');
       env = ENV;
    } else {
        const {ENV} =  await import('./browser/browser-env-prod.js');
        env = ENV;
    }
    env.load = async function(url){

        console.log("load env "+url);
        const absolute = new URL(url,location);
        env.drHostEnv = await import(absolute);
        if (env.drHostEnv) {
            env.drHostEnv.configure(env);
        }
    }
    
} else {
    // node.js
    env = process.env;
}




export default env;