const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const const_module = require('./const')
const { v4: uuidv4 } = require('uuid')
const emailModule = require('./email')
const redis_moudle = require('./redis')


async function GetVarifyCode(call, callback) {
    console.log("email is ", call.request.email)
    try {
        let get_result = await redis_moudle.GetRedis(const_module.code_prefix + call.request.email);
        let uniqueId = get_result;
        if(get_result == null){
            uniqueId = uuidv4();
            if(uniqueId.length > 4){
                uniqueId = uniqueId.substring(0, 4);
            }
            console.log("uuid is: ", uniqueId);
            let b_res = await redis_moudle.SetRedisExpire(const_module.code_prefix + call.request.email, uniqueId, 600);
            if(!b_res){
                callback(null, {
                    email: call.request.email,
                    error: const_module.Errors.RedisErr
                });
                return;
            }
        }
        console.log("uniqueId is ", uniqueId)
        
        let text_str = '您的验证码为' + uniqueId + '请10分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: '17679103695@163.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };

        let send_res = await emailModule.SendMail(mailOptions);
        console.log("send res is ", send_res)
        if(!send_res){
            callback(null, {
                email: call.request.email,
                error: const_module.Errors.SendErr
            });
            return;
        }

        callback(null, {
            email: call.request.email,
            error: const_module.Errors.Success
        });


    } catch (error) {
        console.log("catch error is ", error)

        callback(null, {
            email: call.request.email,
            error: const_module.Errors.Exception
        });
    }

}


function main() {
    //console.log(uuidv4());
    const grpc = require('@grpc/grpc-js')
    var server = new grpc.Server()
    server.addService(message_proto.VarifyService.service, { GetVarifyCode: GetVarifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        server.start()
        console.log('grpc server started')
        process.on('SIGINT', () => {
            console.log('Received SIGINT signal');
            // 在这里执行需要在退出前完成的操作
          
            // 退出进程
            process.exit(0);
          })
    })
}

main()
