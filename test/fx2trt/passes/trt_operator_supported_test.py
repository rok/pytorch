# Owner(s): ["oncall: fx"]

import unittest
import torch.fx.experimental.fx_acc.acc_ops  # noqa: F401
import torch
import torch.fx
from torch.fx.experimental.fx2trt.tools.trt_splitter import create_trt_operator_support
import torch.nn as nn
from torch.fx.experimental.fx_acc import acc_ops, acc_tracer


class TestTRTOperatorSupport(unittest.TestCase):
    def test_supported_node_target(self):
        class TestModule(nn.Module):
            def __init__(self):
                super().__init__()
                self.linear = nn.Linear(1, 1)

            def forward(self, x):
                x = self.linear(x)
                x = x + 1
                return torch.add(input=x, other=x)

        mod = TestModule()
        traced_mod = acc_tracer.trace(mod, torch.randn(1, 2, 1, 1))
        op_support = create_trt_operator_support()
        for node in traced_mod.graph.nodes:
            self.assertTrue(op_support.is_node_supported(mod, node))


    def test_unsupport_node_explicit_batch_dim(self):
        class TestModule(nn.Module):
            def forward(self, x):
                y = torch.add(input=x, other=x)
                return torch.split(y, 2)

        mod = TestModule()
        traced_mod = acc_tracer.trace(mod, torch.randn(5, 2))
        op_support = create_trt_operator_support(use_implicit_batch_dim=False)

        for node in traced_mod.graph.nodes:
            if node.target == acc_ops.add:
                self.assertTrue(op_support.is_node_supported(mod, node))
            elif node.target == acc_ops.split:
                self.assertFalse(op_support.is_node_supported(mod, node))


    def test_unsupport_node_implicit_batch_dim(self):
        class TestModule(nn.Module):
            def forward(self, x):
                y = torch.add(input=x, other=x)
                return nn.functional.gelu(y)

        mod = TestModule()
        traced_mod = acc_tracer.trace(mod, torch.randn(5, 2))
        op_support = create_trt_operator_support(use_implicit_batch_dim=True)

        for node in traced_mod.graph.nodes:
            if node.target == acc_ops.add:
                self.assertTrue(op_support.is_node_supported(mod, node))
            elif node.target == acc_ops.gelu:
                self.assertFalse(op_support.is_node_supported(mod, node))