import { DataSegment } from "quantifio-proxy";

export interface IDecoder{
    Decode(segments: Iterable<DataSegment>): any;
}