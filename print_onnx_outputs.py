import onnx
model = onnx.load("models/ctdet_bdd_resnet18_384.onnx")
print("Inputs:")
for inp in model.graph.input:
    print(inp.name)
print("Outputs:")
for out in model.graph.output:
    print(out.name)
