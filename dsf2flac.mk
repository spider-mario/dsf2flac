##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=dsf2flac
ConfigurationName      :=Release
WorkspacePath          := "/home/jacko/programming/codelite"
ProjectPath            := "/home/jacko/programming/codelite/dsf2pcm"
IntermediateDirectory  :=./Release
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Jack Oclee-Brown
Date                   :=18/03/13
CodeLitePath           :="/home/jacko/.codelite"
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="dsf2flac.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -static
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)./src/ 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)FLAC++ $(LibrarySwitch)FLAC $(LibrarySwitch)boost_system $(LibrarySwitch)boost_timer $(LibrarySwitch)boost_filesystem $(LibrarySwitch)boost_chrono $(LibrarySwitch)id3 $(LibrarySwitch)z $(LibrarySwitch)rt $(LibrarySwitch)ogg 
ArLibs                 :=  "FLAC++" "FLAC" "boost_system" "boost_timer" "boost_filesystem" "boost_chrono" "id3" "z" "rt" "ogg" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -O3 -Wall $(Preprocessors)
CFLAGS   :=  -O3 -Wall $(Preprocessors)


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/src_cmdline$(ObjectSuffix) $(IntermediateDirectory)/src_dsd_decimator$(ObjectSuffix) $(IntermediateDirectory)/src_dsd_sample_reader$(ObjectSuffix) $(IntermediateDirectory)/src_dsf_file_reader$(ObjectSuffix) $(IntermediateDirectory)/src_filters$(ObjectSuffix) $(IntermediateDirectory)/src_main$(ObjectSuffix) $(IntermediateDirectory)/src_dsdiff_file_reader$(ObjectSuffix) $(IntermediateDirectory)/src_fstream_plus$(ObjectSuffix) 

Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@test -d ./Release || $(MakeDirCommand) ./Release

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_cmdline$(ObjectSuffix): ./src/cmdline.c $(IntermediateDirectory)/src_cmdline$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/jacko/programming/dsf2pcm_svn/src/cmdline.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_cmdline$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_cmdline$(DependSuffix): ./src/cmdline.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_cmdline$(ObjectSuffix) -MF$(IntermediateDirectory)/src_cmdline$(DependSuffix) -MM "./src/cmdline.c"

$(IntermediateDirectory)/src_cmdline$(PreprocessSuffix): ./src/cmdline.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_cmdline$(PreprocessSuffix) "./src/cmdline.c"

$(IntermediateDirectory)/src_dsd_decimator$(ObjectSuffix): ./src/dsd_decimator.cpp $(IntermediateDirectory)/src_dsd_decimator$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/jacko/programming/dsf2pcm_svn/src/dsd_decimator.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_dsd_decimator$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_dsd_decimator$(DependSuffix): ./src/dsd_decimator.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_dsd_decimator$(ObjectSuffix) -MF$(IntermediateDirectory)/src_dsd_decimator$(DependSuffix) -MM "./src/dsd_decimator.cpp"

$(IntermediateDirectory)/src_dsd_decimator$(PreprocessSuffix): ./src/dsd_decimator.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_dsd_decimator$(PreprocessSuffix) "./src/dsd_decimator.cpp"

$(IntermediateDirectory)/src_dsd_sample_reader$(ObjectSuffix): ./src/dsd_sample_reader.cpp $(IntermediateDirectory)/src_dsd_sample_reader$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/jacko/programming/dsf2pcm_svn/src/dsd_sample_reader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_dsd_sample_reader$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_dsd_sample_reader$(DependSuffix): ./src/dsd_sample_reader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_dsd_sample_reader$(ObjectSuffix) -MF$(IntermediateDirectory)/src_dsd_sample_reader$(DependSuffix) -MM "./src/dsd_sample_reader.cpp"

$(IntermediateDirectory)/src_dsd_sample_reader$(PreprocessSuffix): ./src/dsd_sample_reader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_dsd_sample_reader$(PreprocessSuffix) "./src/dsd_sample_reader.cpp"

$(IntermediateDirectory)/src_dsf_file_reader$(ObjectSuffix): ./src/dsf_file_reader.cpp $(IntermediateDirectory)/src_dsf_file_reader$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/jacko/programming/dsf2pcm_svn/src/dsf_file_reader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_dsf_file_reader$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_dsf_file_reader$(DependSuffix): ./src/dsf_file_reader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_dsf_file_reader$(ObjectSuffix) -MF$(IntermediateDirectory)/src_dsf_file_reader$(DependSuffix) -MM "./src/dsf_file_reader.cpp"

$(IntermediateDirectory)/src_dsf_file_reader$(PreprocessSuffix): ./src/dsf_file_reader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_dsf_file_reader$(PreprocessSuffix) "./src/dsf_file_reader.cpp"

$(IntermediateDirectory)/src_filters$(ObjectSuffix): ./src/filters.cpp $(IntermediateDirectory)/src_filters$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/jacko/programming/dsf2pcm_svn/src/filters.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_filters$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_filters$(DependSuffix): ./src/filters.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_filters$(ObjectSuffix) -MF$(IntermediateDirectory)/src_filters$(DependSuffix) -MM "./src/filters.cpp"

$(IntermediateDirectory)/src_filters$(PreprocessSuffix): ./src/filters.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_filters$(PreprocessSuffix) "./src/filters.cpp"

$(IntermediateDirectory)/src_main$(ObjectSuffix): ./src/main.cpp $(IntermediateDirectory)/src_main$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/jacko/programming/dsf2pcm_svn/src/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_main$(DependSuffix): ./src/main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_main$(ObjectSuffix) -MF$(IntermediateDirectory)/src_main$(DependSuffix) -MM "./src/main.cpp"

$(IntermediateDirectory)/src_main$(PreprocessSuffix): ./src/main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_main$(PreprocessSuffix) "./src/main.cpp"

$(IntermediateDirectory)/src_dsdiff_file_reader$(ObjectSuffix): ./src/dsdiff_file_reader.cpp $(IntermediateDirectory)/src_dsdiff_file_reader$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/jacko/programming/dsf2pcm_svn/src/dsdiff_file_reader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_dsdiff_file_reader$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_dsdiff_file_reader$(DependSuffix): ./src/dsdiff_file_reader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_dsdiff_file_reader$(ObjectSuffix) -MF$(IntermediateDirectory)/src_dsdiff_file_reader$(DependSuffix) -MM "./src/dsdiff_file_reader.cpp"

$(IntermediateDirectory)/src_dsdiff_file_reader$(PreprocessSuffix): ./src/dsdiff_file_reader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_dsdiff_file_reader$(PreprocessSuffix) "./src/dsdiff_file_reader.cpp"

$(IntermediateDirectory)/src_fstream_plus$(ObjectSuffix): ./src/fstream_plus.cpp $(IntermediateDirectory)/src_fstream_plus$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/jacko/programming/dsf2pcm_svn/src/fstream_plus.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_fstream_plus$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_fstream_plus$(DependSuffix): ./src/fstream_plus.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_fstream_plus$(ObjectSuffix) -MF$(IntermediateDirectory)/src_fstream_plus$(DependSuffix) -MM "./src/fstream_plus.cpp"

$(IntermediateDirectory)/src_fstream_plus$(PreprocessSuffix): ./src/fstream_plus.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_fstream_plus$(PreprocessSuffix) "./src/fstream_plus.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/src_cmdline$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_cmdline$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_cmdline$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_dsd_decimator$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_dsd_decimator$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_dsd_decimator$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_dsd_sample_reader$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_dsd_sample_reader$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_dsd_sample_reader$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_dsf_file_reader$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_dsf_file_reader$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_dsf_file_reader$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_filters$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_filters$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_filters$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_dsdiff_file_reader$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_dsdiff_file_reader$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_dsdiff_file_reader$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/src_fstream_plus$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/src_fstream_plus$(DependSuffix)
	$(RM) $(IntermediateDirectory)/src_fstream_plus$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) "../.build-release/dsf2flac"


