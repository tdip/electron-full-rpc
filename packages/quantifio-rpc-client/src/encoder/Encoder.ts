import { DataSegment } from "quantifio-proxy";

import { IEncoder } from "./IEncoder";

export class Encoder implements IEncoder{

    Encode(input): Iterable<DataSegment>{
        return [
            {
                path: [],
                part: JSON.stringify("soy el kaiser"),
                metadata: {type: {remoteValue: {typeId: "string"}}}
            }
        ];
    }
}