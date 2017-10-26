export declare type RpcRemoteBase = {
    readonly remoteObjectId: string
}

export declare type RpcRemoteProperty = RpcRemoteBase & {
    readonly remotePropertyId: string
}

export declare type RpcRemoteMethod = RpcRemoteBase & {
    readonly remoteMethodId: string
}

export declare type RpcRemotePropertySelector = {
    readonly remoteProperty: RpcRemoteProperty
}

export declare type RpcRemoteMethodSelector = {
    readonly remoteMethod: RpcRemoteMethod
}

export declare type RpcRemoteValue = {
    readonly typeId: string
}

export declare type RpcObjectValueSelector = {
    readonly remoteValue: RpcRemoteValue
}

export declare type IRpcMessage = {
    readonly rpceSourceClient: string;
    readonly rpcLabel: string;
    readonly rpcMessageId: string;
    readonly rpcMessageType: string;
}

export declare type  IRpcSegmentMessage = IRpcMessage & {
    readonly complete: boolean;
}

export declare type IRpcRemoteRequest = IRpcMessage & {
    readonly rpcTargetClient: string;
    readonly rpcTargetObject: string;
}

export declare type IRpcRemoteMethodCall = IRpcRemoteRequest & {
    readonly rpcArgsChannels: string[];
    readonly rpcResultChannel: string;
}

export declare type RpcValueType = RpcObjectValueSelector
    | RpcRemoteMethodSelector
    | RpcRemotePropertySelector;

export declare type RpcValueMetadata = {
    readonly type: RpcValueType
}

export declare type DataSegment = {
    /**
     * Metadata that will be used to
     * reconstruct the value of this field
     */
    readonly metadata?: RpcValueMetadata,
    
    /**
     * A piece of data that will be used to reconstruct
     * the orivinal object encoded as a string with
     * the Object.Stringify method.
     */
    readonly part?: string,
    
    /**
     * The path that corresponds to the location of the
     * property being updated. Empty path refers to
     * the root object.
     */
    readonly path: string[]
}

export declare type IDataSegmentMessage = IRpcSegmentMessage & DataSegment;

export interface IRpcChannelItem<T>{
    readonly value: T
    readonly complete: boolean    
}

export interface  IRpcChannel<T>{
    next(): IRpcChannelItem<T>
}

export class RpcHandler{
    emitData(data: IDataSegmentMessage): void;
    emitRpcCall(message: IRpcRemoteMethodCall): void;
    getData(channelId: string): IRpcChannel<IDataSegmentMessage>;
}