import onnx
import json
model = onnx.load('models/object_detection/ctdet_bdd_resnet18_384.onnx')
out = []
for node in model.graph.node:
    if node.op_type == 'DCNv2' or 'DCN' in node.op_type:
        node_info = {'name': node.name, 'op_type': node.op_type, 'attributes': {}}
        for attr in node.attribute:
            if attr.type == onnx.AttributeProto.INTS:
                node_info['attributes'][attr.name] = list(attr.ints)
            elif attr.type == onnx.AttributeProto.INT:
                node_info['attributes'][attr.name] = attr.i
            else:
                node_info['attributes'][attr.name] = str(attr.type)
        out.append(node_info)
with open('dcn_attrs.json', 'w') as f:
    json.dump(out, f, indent=2)
