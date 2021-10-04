#pragma once

#include "lazy_tensor_core/csrc/ir.h"
#include "lazy_tensors/computation_client/cache.h"
#include "lazy_tensors/shape.h"

namespace torch_lazy_tensors {
namespace ir {

// Helper that makes it easy to access the TsNode::shape() method
// from an ir::Output that holds a Node* that points to a TsNode
lazy_tensors::Shape GetShapeFromTsOutput(const ir::Output& output);
lazy_tensors::Shape GetShapeFromTsValue(const ir::Value& value);
lazy_tensors::Shape GetShapeFromTsNode(const ir::Node& value);
void TsNodeSetShapeDeferred(
    NodePtr node, const std::function<lazy_tensors::Shape()>& shape_fn);

class TsNode : public Node {
 public:
  TsNode(OpKind op, OpList operands, lazy_tensors::Shape shape,
         size_t num_outputs = 1, lazy_tensors::hash_t hash_seed = kHashSeed);

  // Same as the constructor above, but the shape is generated by a function,
  // only if needed (shape cache miss).
  TsNode(OpKind op, OpList operands,
         const std::function<lazy_tensors::Shape()>& shape_fn,
         size_t num_outputs = 1, lazy_tensors::hash_t hash_seed = kHashSeed);

  // The shape is set later.
  TsNode(OpKind op, OpList operands, size_t num_outputs = 1,
         lazy_tensors::hash_t hash_seed = kHashSeed);

  void SetShapeDeferred(const std::function<lazy_tensors::Shape()>& shape_fn);

  // Contructor used to create leaf nodes.
  TsNode(OpKind op, lazy_tensors::Shape shape, size_t num_outputs,
         lazy_tensors::hash_t hash_seed);

  virtual ~TsNode() {}

  lazy_tensors::Shape GetOpShape(
      const std::function<lazy_tensors::Shape()>& shape_fn) const;

  // Retrieves the full shape of the IR Node. Note that if this is a
  // multi-output node, the returned shape will be a tuple.
  const lazy_tensors::Shape& shape() const;

  // Retrieves the shape of the output at a given index. If the node is not a
  // multi-output node, output_index must be zero.
  const lazy_tensors::Shape& shape(size_t output_index) const;

  virtual std::string ToString() const;

  static lazy_tensors::hash_t GetOpHash(OpKind op,
                                        const lazy_tensors::Shape& shape,
                                        lazy_tensors::hash_t hash_seed);

 private:
  lazy_tensors::Shape shape_;
};

}  // namespace ir
}  // namespace torch_lazy_tensors