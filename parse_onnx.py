import onnx
model = onnx.load('models/object_detection/ctdet_bdd_resnet18_384.onnx')
ops = set()
for node in model.graph.node:
    ops.add(node.op_type)
print(ops)
