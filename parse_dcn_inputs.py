import onnx
import json
model = onnx.load('models/object_detection/ctdet_bdd_resnet18_384.onnx')
out = []
for node in model.graph.node:
    if node.op_type == 'DCNv2':
        out.append({'name': node.name, 'inputs': list(node.input)})
print(json.dumps(out, indent=2))
