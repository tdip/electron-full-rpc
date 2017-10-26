import { DataSegment } from "quantifio-proxy";

export interface IEncoder{
    Encode(value: any): Iterable<DataSegment>
}