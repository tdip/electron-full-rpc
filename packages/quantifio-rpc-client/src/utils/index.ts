import {
    RpcObjectValueSelector,
    RpcRemoteMethodSelector,
    RpcRemotePropertySelector
} from "quantifio-proxy";
import uuidv4 = require("uuid/v4");

export function isRpcRemoteProperty(obj: {}[]): obj is [RpcRemotePropertySelector]{
    return obj[0] && (<RpcRemotePropertySelector>obj[0]).remoteProperty != undefined;
}

export function isRpcRemoteFunction(obj: {}[]): obj is [RpcRemoteMethodSelector]{
    return obj[0] && (<RpcRemoteMethodSelector>obj[0]).remoteMethod !== undefined;
}

export function uuid(): string{
    return uuidv4();
}