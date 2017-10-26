import {
    IDataSegmentMessage,
    IRpcRemoteMethodCall,
    RpcHandler } from "quantifio-proxy";

import { Decoder, IDecoder } from "../decoder";
import { Encoder, IEncoder } from "../encoder";
import { uuid } from "../utils";

export type RpcRemoteMethodCallArgs = {
    readonly clientId: string,
    readonly objectId: string
}

export class RpcClient{
    private readonly handler: RpcHandler;
    private readonly decoder: IDecoder;
    private readonly encoder: IEncoder;
    private readonly clientId: string;
    constructor(){
        this.handler = new RpcHandler();
        this.encoder = new Encoder();
        this.clientId = uuid();
        this.decoder = new Decoder();
    }

    RpcCallRemoteMethod(rpc: RpcRemoteMethodCallArgs, ...args: any[]): any{
        // Create a unique id for the channels where arguments
        // will be sent.
        const rpcArgsChannels = args.map(_ => uuid());
        const rpcResultChannel = uuid();
        const rpceSourceClient = this.clientId;
        const rpcCall: IRpcRemoteMethodCall = {
            rpcMessageId: uuid(),
            rpcArgsChannels,
            rpceSourceClient,
            rpcLabel: uuid(),
            // Gets set to the correct label by the RpcHandler
            rpcMessageType: undefined,
            rpcResultChannel,
            rpcTargetClient: rpc.clientId,
            rpcTargetObject: rpc.objectId
        };

        this.handler.emitRpcCall(rpcCall);
        for(let i = 0; i < args.length; i++){
            let arg = args[i];
            let rpcLabel = rpcArgsChannels[i];
            let encoded = this.encoder.Encode(arg);
            for(let chunk of encoded){
                let part: IDataSegmentMessage = Object.assign(
                    chunk,
                    {
                        complete: false,
                        // will be set to the correct type by the Handler
                        rpcMessageType: undefined,
                        rpcMessageId: uuid(),
                        rpcLabel,
                        rpceSourceClient
                    });
                this.handler.emitData(part);
            }
        }
        
        const channel = this.handler.getData(rpcResultChannel);
        function* segments(){
            do{
                var segment = channel.next();
                if(segment.value){
                    yield segment.value;
                }
            }while(!segment.complete);
        }

        return this.decoder.Decode(segments());
    }
}