//===-- MCSubtargetInfo.cpp - Subtarget Information -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

using namespace llvm_ks;

static FeatureBitset getFeatures(StringRef CPU, StringRef FS,
                                 ArrayRef<SubtargetFeatureKV> ProcDesc,
                                 ArrayRef<SubtargetFeatureKV> ProcFeatures) {
  SubtargetFeatures Features(FS);
  return Features.getFeatureBits(CPU, ProcDesc, ProcFeatures);
}

void MCSubtargetInfo::InitMCProcessorInfo(StringRef CPU, StringRef FS) {
  FeatureBits = getFeatures(CPU, FS, ProcDesc, ProcFeatures);
  if (!CPU.empty() && ProcSchedModels)
      CPUSchedModel = &getSchedModelForCPU(CPU);
}

void MCSubtargetInfo::setDefaultFeatures(StringRef CPU, StringRef FS) {
  FeatureBits = getFeatures(CPU, FS, ProcDesc, ProcFeatures);
}

MCSubtargetInfo::MCSubtargetInfo(
    const Triple &TT, StringRef C, StringRef FS,
    ArrayRef<SubtargetFeatureKV> PF, ArrayRef<SubtargetFeatureKV> PD,
    const SubtargetInfoKV *ProcSched)
    : TargetTriple(TT), CPU(C), ProcFeatures(PF), ProcDesc(PD),
    ProcSchedModels(ProcSched) {
  InitMCProcessorInfo(CPU, FS);
}

/// ToggleFeature - Toggle a feature and returns the re-computed feature
/// bits. This version does not change the implied bits.
FeatureBitset MCSubtargetInfo::ToggleFeature(uint64_t FB) {
  FeatureBits.flip(FB);
  return FeatureBits;
}

FeatureBitset MCSubtargetInfo::ToggleFeature(const FeatureBitset &FB) {
  FeatureBits ^= FB;
  return FeatureBits;
}

/// ToggleFeature - Toggle a feature and returns the re-computed feature
/// bits. This version will also change all implied bits.
FeatureBitset MCSubtargetInfo::ToggleFeature(StringRef FS) {
  SubtargetFeatures::ToggleFeature(FeatureBits, FS, ProcFeatures);
  return FeatureBits;
}

FeatureBitset MCSubtargetInfo::ApplyFeatureFlag(StringRef FS) {
  SubtargetFeatures::ApplyFeatureFlag(FeatureBits, FS, ProcFeatures);
  return FeatureBits;
}

const MCSchedModel &MCSubtargetInfo::getSchedModelForCPU(StringRef CPU) const {
  assert(ProcSchedModels && "Processor machine model not available!");

  ArrayRef<SubtargetInfoKV> SchedModels(ProcSchedModels, ProcDesc.size());

  assert(std::is_sorted(SchedModels.begin(), SchedModels.end(),
                    [](const SubtargetInfoKV &LHS, const SubtargetInfoKV &RHS) {
                      return strcmp(LHS.Key, RHS.Key) < 0;
                    }) &&
         "Processor machine model table is not sorted");

  // Find entry
  auto Found =
    std::lower_bound(SchedModels.begin(), SchedModels.end(), CPU);
#if 0
  if (Found == SchedModels.end() || StringRef(Found->Key) != CPU) {
    if (CPU != "help") // Don't error if the user asked for help.
      errs() << "'" << CPU
             << "' is not a recognized processor for this target"
             << " (ignoring processor)\n";
    return MCSchedModel::GetDefaultSchedModel();
  }
#endif
  assert(Found->Value && "Missing processor SchedModel value");
  return *(const MCSchedModel *)Found->Value;
}

