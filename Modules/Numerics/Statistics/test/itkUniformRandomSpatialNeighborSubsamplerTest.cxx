/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkWin32Header.h"

#include <fstream>

#include "itkImageToNeighborhoodSampleAdaptor.h"
#include "itkSubsample.h"
#include "itkImageFileWriter.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"

#include "itkUniformRandomSpatialNeighborSubsampler.h"
#include "itkTestingMacros.h"

int
itkUniformRandomSpatialNeighborSubsamplerTest(int argc, char * argv[])
{
  if (argc < 2)
  {
    std::cerr << "Missing Parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv) << " useClockForSeed [outFileName]" << std::endl;
    return EXIT_FAILURE;
  }

  constexpr unsigned int Dimension = 2;

  using FloatImage = itk::Image<float, Dimension>;
  using RegionType = FloatImage::RegionType;
  using IndexType = FloatImage::IndexType;
  using SizeType = FloatImage::SizeType;
  using BoundaryCondition = itk::ZeroFluxNeumannBoundaryCondition<FloatImage>;
  using AdaptorType = itk::Statistics::ImageToNeighborhoodSampleAdaptor<FloatImage, BoundaryCondition>;
  using SamplerType = itk::Statistics::UniformRandomSpatialNeighborSubsampler<AdaptorType, RegionType>;
  using WriterType = itk::ImageFileWriter<FloatImage>;

  auto     inImage = FloatImage::New();
  SizeType sz;
  sz.Fill(35);
  IndexType idx;
  idx.Fill(0);
  RegionType region;
  region.SetSize(sz);
  region.SetIndex(idx);

  inImage->SetRegions(region);
  inImage->Allocate(true); // initialize buffer to zero

  auto sample = AdaptorType::New();
  sample->SetImage(inImage);

  auto samplerOrig = SamplerType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(samplerOrig, UniformRandomSpatialNeighborSubsampler, SpatialNeighborSubsampler);

  samplerOrig->SetSample(sample);
  samplerOrig->SetSampleRegion(region);
  samplerOrig->SetRadius(20);
  samplerOrig->SetNumberOfResultsRequested(50);
  samplerOrig->SetSeed(100);
  samplerOrig->CanSelectQueryOff();


  auto useClockForSeed = static_cast<bool>(std::stoi(argv[1]));
  ITK_TEST_SET_GET_BOOLEAN(samplerOrig, UseClockForSeed, useClockForSeed);

  // Test clone mechanism
  SamplerType::Pointer sampler = samplerOrig->Clone().GetPointer();

  ITK_TEST_SET_GET_VALUE(samplerOrig->GetSample(), sampler->GetSample());
  ITK_TEST_SET_GET_VALUE(samplerOrig->GetSampleRegion(), sampler->GetSampleRegion());
  ITK_TEST_SET_GET_VALUE(samplerOrig->GetRadius(), sampler->GetRadius());
  ITK_TEST_SET_GET_VALUE(samplerOrig->GetNumberOfResultsRequested(), sampler->GetNumberOfResultsRequested());
  ITK_TEST_SET_GET_VALUE(samplerOrig->GetSeed(), sampler->GetSeed());
  ITK_TEST_SET_GET_VALUE(samplerOrig->GetCanSelectQuery(), sampler->GetCanSelectQuery());

  SamplerType::SubsamplePointer subsample = SamplerType::SubsampleType::New();
  sampler->Search(612, subsample);

  for (SamplerType::SubsampleConstIterator sIt = subsample->Begin(); sIt != subsample->End(); ++sIt)
  {
    IndexType index;
    index = sIt.GetMeasurementVector()[0].GetIndex();
    inImage->SetPixel(index, 255);
  }


  if (argc > 2)
  {
    const std::string outFileName(argv[2]);

    auto writer = WriterType::New();
    writer->SetFileName(outFileName);
    writer->SetInput(inImage);

    ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());
  }

  std::cout << "Test finished." << std::endl;
  return EXIT_SUCCESS;
}
