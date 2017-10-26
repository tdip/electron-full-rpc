import { Map, Record } from "immutable";
import {
    DataSegment,
    IDataSegmentMessage,
    IRpcChannel } from "quantifio-proxy";

import {
    isRpcRemoteFunction,
    isRpcRemoteProperty
 } from "../utils";

import { IDecoder } from "./IDecoder";

const selfSection = "self";
const propsSection = "props";

export type SelfSection = {
    baseValue: {}[]
}

const selfSectionDef: SelfSection = {
        baseValue: []
    };



function updateEntry(part: IDataSegmentMessage, section: SelfSection){

    const updates: Partial<SelfSection> = {};

    if(part){
        updates.baseValue = section.baseValue.concat("");
    }

    return Object.assign(
        {},
        section,
        updates
    );
}

/**
 * Return a map which will be used as a template to recunsruct
 * the object being received via rpc. It is composed of the
 * self section and the props section.
 * The self section will collect all values that will be used
 * to construct an object that represents a remote object.
 * The props section will list the properties that the remote
 * object will have. Those properties will again refer to maps
 * with the same structure as this map. Unfortunately, this
 * type cannot be represented by the typescript type system.
 */
function getEntry(){
    return Map<string,{}>([
        [selfSection, Object.assign({}, selfSectionDef)],
        [propsSection, Map<string,{}>()]
    ]);
}

function getPath(path: string[]){
    const result = [];

    path.forEach(part => result.splice(result.length, 0, propsSection, part));
    path.push(selfSection);

    return result;
}

export class Decoder implements IDecoder{

    Decode(parts: Iterable<DataSegment>){
        return "This is the decode";
    }

    _Decode(parts: IRpcChannel<IDataSegmentMessage>){
        const obj = getEntry().asMutable();
        
        var part = parts.next();
        do{
            let value = part.value;
            if(!value){
                continue;
            }

            obj.updateIn(
                getPath(value.path),
                selfSectionDef,
                self => updateEntry(value, self)
            );

        }while(!part.complete);

        return obj;
    }

    private createBaseObject(self: SelfSection){

        const baseValue = self.baseValue;
        if(isRpcRemoteFunction(baseValue)){
            let remoteMethod = baseValue[0].remoteMethod;
            
        }

        return {};
    }

    private Construct(parts: Map<string, {}>){
        const self = parts.get(selfSection) as SelfSection;
        const base = this.createBaseObject(self);
        
        const props = parts.get(propsSection) as Map<string, Map<string,{}>>;

        for(let key of props.keys()){
            let value = this.Construct(props.get(key));
            Object.defineProperty(
                self,
                key,
                {
                    get(){return value;}
                }
            )
        }

        return self;
    }

}