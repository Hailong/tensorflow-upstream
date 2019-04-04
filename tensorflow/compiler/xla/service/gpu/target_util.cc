/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
// Provide helper routine for obtaining  gpu target information useful
// for llvm IR contruction.

#include "tensorflow/compiler/xla/service/gpu/target_util.h"

#include "llvm/IR/MDBuilder.h"
#include "absl/strings/str_cat.h"
#include "tensorflow/compiler/xla/primitive_util.h"
#include "tensorflow/compiler/xla/service/llvm_ir/ir_builder_mixin.h"
#include "tensorflow/compiler/xla/service/llvm_ir/llvm_util.h"
#include "tensorflow/compiler/xla/xla_data.pb.h"
#include "tensorflow/core/platform/logging.h"



namespace xla {

using absl::StrAppend;

namespace gpu {
namespace {
// Utility functions to obtain NVPTX/AMDGPU specific information.

struct TargetInfo {
  llvm::Intrinsic::ID intrinsic;
  const string callee_name;
  absl::Span<const PrimitiveType> input_types;
  PrimitiveType output_type;
};


// Wrapper structure for carrying llvm intrinsic ids for NVPTX/AMDGPU platforms.
struct MultipleTargetInfo {
  struct TargetInfo nvptx_info;
  struct TargetInfo amdgpu_info;
};

// Gets the llvm intrinsic ids on different platforms (NVPTX, AMDGPU)
// corresponding to the give TargetIntrinsicID.
struct MultipleTargetInfo GetTargetInfo(TargetFunctionID function_id) {
  switch (function_id) {
    case TargetFunctionID::kShflDownF32: {
      return {
              {
                llvm::Intrinsic::nvvm_shfl_sync_down_f32,"",
                { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID,
              },
              {
                llvm::Intrinsic::not_intrinsic, "__ockl_readuplane", 
                {PRIMITIVE_TYPE_INVALID, S32, S32, PRIMITIVE_TYPE_INVALID}, S32,
              },
           };
    }
    case TargetFunctionID::kShflDownI32: {
      return {
              {
                llvm::Intrinsic::nvvm_shfl_sync_down_i32,"",
                { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
              },
              {
                llvm::Intrinsic::not_intrinsic, "__ockl_readuplane", 
                {PRIMITIVE_TYPE_INVALID, S32, S32, PRIMITIVE_TYPE_INVALID}, S32,
              },
           };
    }
    case TargetFunctionID::kThreadIdx: {
      return {
              {
                llvm::Intrinsic::nvvm_read_ptx_sreg_tid_x,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
              },
              {
                llvm::Intrinsic::amdgcn_workitem_id_x,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
              }
           };
    }
    case TargetFunctionID::kThreadIdy: {
      return {
              {
                llvm::Intrinsic::nvvm_read_ptx_sreg_tid_y,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
              },
              {
                llvm::Intrinsic::amdgcn_workitem_id_y,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
             }
      };
    }
    case TargetFunctionID::kThreadIdz: {
      return {
              {
                llvm::Intrinsic::nvvm_read_ptx_sreg_tid_x,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
              },
              { 
                llvm::Intrinsic::amdgcn_workitem_id_x,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
             }
      };
    }
    case TargetFunctionID::kBlockIdx: {
      return {
              {
                llvm::Intrinsic::nvvm_read_ptx_sreg_ctaid_x,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
              },
             { 
                llvm::Intrinsic::amdgcn_workgroup_id_x,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
             }
      };
    }
    case TargetFunctionID::kBlockIdy: {
      return {
              {
                llvm::Intrinsic::nvvm_read_ptx_sreg_ctaid_y,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
              },
              {
                llvm::Intrinsic::amdgcn_workgroup_id_y,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
             }
     };
    }
    case TargetFunctionID::kBlockIdz: {
      return {
              {
                llvm::Intrinsic::nvvm_read_ptx_sreg_ctaid_z,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
              },
              {
                llvm::Intrinsic::amdgcn_workgroup_id_z,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
             }
      };
    }
    case TargetFunctionID::kBarrierId: {
      return {
              {
                llvm::Intrinsic::nvvm_barrier0,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
              },
              {
                llvm::Intrinsic::amdgcn_s_barrier,
                "", { PRIMITIVE_TYPE_INVALID },PRIMITIVE_TYPE_INVALID, 
              }
      };
    }
  }
}
}  // namespace

llvm::Value* EmitCallToTargetFunction(
    TargetFunctionID function_id, absl::Span<llvm::Value* const> operands,
    absl::Span<const PrimitiveType> input_types, PrimitiveType output_type,
    absl::Span<const llvm::Attribute::AttrKind> attributes,
    absl::Span<llvm::Type* const> overloaded_types,
    llvm::IRBuilder<>* b) {
  llvm::Module* module = b->GetInsertBlock()->getModule();
  struct MultipleTargetInfo all_gpu_info = GetTargetInfo(function_id);
  llvm::Triple target_triple = llvm::Triple(module->getTargetTriple());
  struct TargetInfo* gpu_info;

  if ((target_triple.getArch() == llvm::Triple::nvptx) ||
      (target_triple.getArch() == llvm::Triple::nvptx64)) {
    gpu_info  = &(all_gpu_info.nvptx_info);
  } else if (target_triple.getArch() == llvm::Triple::amdgcn) {
    gpu_info  = &(all_gpu_info.amdgpu_info);
  } else {
    LOG(FATAL) << "Invalid triple " << target_triple.str();
  }

  if (gpu_info->intrinsic != llvm::Intrinsic::not_intrinsic){
    llvm::Function* intrinsic = llvm::Intrinsic::getDeclaration(
      module, gpu_info->intrinsic, llvm_ir::AsArrayRef(overloaded_types));
    return b->CreateCall(intrinsic, llvm_ir::AsArrayRef(operands));
  }
  else { 
    std::vector<llvm::Value*> converted_operands;
    std::vector<llvm::Type*> ir_input_types;
    auto indices = gpu_info->input_types.size();
    PrimitiveType from_type, to_type;
    CHECK_EQ(input_types.size(), gpu_info->input_types.size());
    CHECK_EQ(input_types.size(), operands.size());
    for (unsigned int index = 0; index < operands.size(); ++index){
     to_type = gpu_info->input_types[index];
     from_type = input_types[index];
    if (to_type == PRIMITIVE_TYPE_INVALID)
      continue;
     if (from_type == to_type){
	converted_operands.push_back(const_cast<llvm::Value*>(operands[index]));
     }
     else if( primitive_util::IsFloatingPointType(from_type) && 
             primitive_util::IsSignedIntegralType(to_type) ) {
        converted_operands.push_back(b->CreateFPToSI(operands[index],
                      llvm_ir::PrimitiveTypeToIrType(to_type, module)));
      }
      else {
       LOG(FATAL) << "unhandled conversion operation from " << PrimitiveType_Name(from_type) << "to" << PrimitiveType_Name(to_type);
      }
      ir_input_types.push_back(
          llvm_ir::PrimitiveTypeToIrType(to_type, module));
    }
    llvm::FunctionType* callee_type = llvm::FunctionType::get(
        llvm_ir::PrimitiveTypeToIrType(output_type, module),  // Return type.
       ir_input_types,                                       // Parameter types.
        false);  // No variadic arguments.

   string munged_callee = gpu_info->callee_name;
   switch (output_type) {
    case S32:
      StrAppend(&munged_callee, "_i32");
      break;
    case S64:
      StrAppend(&munged_callee, "_i64");
      break;
    case F32:
      StrAppend(&munged_callee, "_f32");
      break;
    case F64:
      StrAppend(&munged_callee, "_f64");
      break;
    default:
       LOG(FATAL) << "Bad Type " << PrimitiveType_Name(output_type) << "\n";
   }
    // Declares the callee if it is not declared already.
    llvm::Function* callee = llvm::dyn_cast<llvm::Function>(
            b->GetInsertBlock()->getModule()->getOrInsertFunction(
            llvm_ir::AsStringRef(gpu_info->callee_name), callee_type).getCallee());
    for (auto attribute : attributes) {
      callee->addFnAttr(attribute);
    }
    llvm::Value* result =  b->CreateCall(callee, llvm_ir::AsArrayRef(converted_operands));

    from_type = gpu_info->output_type;
    to_type = output_type;
    if (from_type == to_type){
      return result;
    }
    else if( primitive_util::IsFloatingPointType(to_type) && 
             primitive_util::IsSignedIntegralType(from_type) ) {
        llvm::Value* converted_result= b->CreateSIToFP(result,
                      llvm_ir::PrimitiveTypeToIrType(to_type, module));
        return converted_result;
    }
   }
  }

}  // namespace gpu
}  // namespace xla
