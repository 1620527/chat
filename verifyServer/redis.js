const config_moudle = require("./config");
const Redis = require("ioredis");


const RedisCli = new Redis({
    host: config_moudle.redis_host,
    port: config_moudle.redis_port,
    password: config_moudle.redis_passwd
})

RedisCli.on("error", function(error){
    console.log("RedisCli connect error.");
    RedisCli.quit();
})

async function GetRedis(key){
    try{
        const result = await RedisCli.get(key);
        if(result === null){
            console.log('result:' , '<'+result+'>' , 'this key cannot be find.');
            return null;
        }
        console.log('result:' , '<'+result+'>' , "get key success.");
        return result;
    }catch(error){
        console.log('Get redis error is ', error);
        return null;
    }
}

async function QueryRedis(key){
    try{
        const result = await RedisCli.exists(key);
        if(result === 0){
            console.log('the value of key is not exist.');
            return result;
        }
        console.log('the value of key is exist.');
        return result;
    }catch(error){
        console.log('Query redis error is:' , error);
        return 0;
    }
}

async function SetRedisExpire(key, value, expireTime){
    try{
        await RedisCli.set(key, value);
        await RedisCli.expire(key, expireTime);
        return true;
    }catch(error){
        console.log("SetRedisExpire error, ", error);
        return false;
    }
}

function Quit(){
    RedisCli.quit();
}

module.exports = {GetRedis, QueryRedis, SetRedisExpire, Quit,};
