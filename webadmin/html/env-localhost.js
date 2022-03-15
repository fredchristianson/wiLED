/* this is an environment file that should vary dending on where it's deployed.
   env-localhost.js is an example that can be copied to other files such as
   env-prod.js and env-dev.js.  
   I am using it by loading a hostname-based file in leds.html
        env.load(`env-${location.hostname}.js`);
   and I have different env-*-.js files for the different servers I deploy to.
*/
export function configure(env) {
    console.log("env-dev");
    
    env.DEBUG = false;
    env.PROD = true;

    env.LED_STRIPS = [
        {name: 'Living Room',host:"lr.localhost"},
        {name: 'Dining Room',host:"dr.localhost"},
        {name: 'Kitchen Cupboard',host:"kc.localhost"},
        {name: 'Kitchen Floor',host:"kf.localhost"},
        {name: 'Lanai',host:"lanai.localhost"},
        {name: 'Hall',host:"hall.localhost"}
    ];
}