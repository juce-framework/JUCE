/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Reads and Writes WAV format audio files.

    @see AudioFormat

    @tags{Audio}
*/
class JUCE_API  WavAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    /** Creates a format object. */
    WavAudioFormat();

    /** Destructor. */
    ~WavAudioFormat() override;

    //==============================================================================
    // BWAV chunk properties:

    static const char* const bwavDescription;       /**< Metadata property name used in BWAV chunks. */
    static const char* const bwavOriginator;        /**< Metadata property name used in BWAV chunks. */
    static const char* const bwavOriginatorRef;     /**< Metadata property name used in BWAV chunks. */
    static const char* const bwavOriginationDate;   /**< Metadata property name used in BWAV chunks. The format should be: yyyy-mm-dd */
    static const char* const bwavOriginationTime;   /**< Metadata property name used in BWAV chunks. The format should be: format is: hh-mm-ss */
    static const char* const bwavCodingHistory;     /**< Metadata property name used in BWAV chunks. */

    /** Metadata property name used in BWAV chunks.
        This is the number of samples from the start of an edit that the
        file is supposed to begin at. Seems like an obvious mistake to
        only allow a file to occur in an edit once, but that's the way
        it is..

        @see AudioFormatReader::metadataValues, createWriterFor
    */
    static const char* const bwavTimeReference;

    /** Utility function to fill out the appropriate metadata for a BWAV file.

        This just makes it easier than using the property names directly, and it
        fills out the time and date in the right format.
    */
    static StringPairArray createBWAVMetadata (const String& description,
                                               const String& originator,
                                               const String& originatorRef,
                                               Time dateAndTime,
                                               int64 timeReferenceSamples,
                                               const String& codingHistory);

    //==============================================================================
    // 'acid' chunk properties:

    static const char* const acidOneShot;           /**< Metadata property name used in acid chunks. */
    static const char* const acidRootSet;           /**< Metadata property name used in acid chunks. */
    static const char* const acidStretch;           /**< Metadata property name used in acid chunks. */
    static const char* const acidDiskBased;         /**< Metadata property name used in acid chunks. */
    static const char* const acidizerFlag;          /**< Metadata property name used in acid chunks. */
    static const char* const acidRootNote;          /**< Metadata property name used in acid chunks. */
    static const char* const acidBeats;             /**< Metadata property name used in acid chunks. */
    static const char* const acidDenominator;       /**< Metadata property name used in acid chunks. */
    static const char* const acidNumerator;         /**< Metadata property name used in acid chunks. */
    static const char* const acidTempo;             /**< Metadata property name used in acid chunks. */

    //==============================================================================
    // INFO chunk properties:

    static const char* const riffInfoArchivalLocation;      /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoArtist;                /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoBaseURL;               /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoCinematographer;       /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoComment;               /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoComment2;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoComments;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoCommissioned;          /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoCopyright;             /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoCostumeDesigner;       /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoCountry;               /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoCropped;               /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoDateCreated;           /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoDateTimeOriginal;      /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoDefaultAudioStream;    /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoDimension;             /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoDirectory;             /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoDistributedBy;         /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoDotsPerInch;           /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoEditedBy;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoEighthLanguage;        /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoEncodedBy;             /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoEndTimecode;           /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoEngineer;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoFifthLanguage;         /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoFirstLanguage;         /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoFourthLanguage;        /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoGenre;                 /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoKeywords;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoLanguage;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoLength;                /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoLightness;             /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoLocation;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoLogoIconURL;           /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoLogoURL;               /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoMedium;                /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoMoreInfoBannerImage;   /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoMoreInfoBannerURL;     /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoMoreInfoText;          /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoMoreInfoURL;           /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoMusicBy;               /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoNinthLanguage;         /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoNumberOfParts;         /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoOrganisation;          /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoPart;                  /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoProducedBy;            /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoProductName;           /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoProductionDesigner;    /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoProductionStudio;      /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoRate;                  /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoRated;                 /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoRating;                /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoRippedBy;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoSecondaryGenre;        /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoSecondLanguage;        /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoSeventhLanguage;       /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoSharpness;             /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoSixthLanguage;         /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoSoftware;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoSoundSchemeTitle;      /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoSource;                /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoSourceFrom;            /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoStarring_ISTR;         /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoStarring_STAR;         /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoStartTimecode;         /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoStatistics;            /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoSubject;               /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoTapeName;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoTechnician;            /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoThirdLanguage;         /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoTimeCode;              /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoTitle;                 /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoTrackNo;               /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoTrackNumber;           /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoURL;                   /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoVegasVersionMajor;     /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoVegasVersionMinor;     /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoVersion;               /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoWatermarkURL;          /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoWrittenBy;             /**< Metadata property name used in INFO chunks. */
    static const char* const riffInfoYear;                  /**< Metadata property name used in INFO chunks. */

    //==============================================================================
    // ASWG chunk properties:

    static const char* const aswgContentType;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgProject;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgOriginator;                /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgOriginatorStudio;          /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgNotes;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgSession;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgState;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgEditor;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgMixer;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgFxChainName;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgChannelConfig;             /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgAmbisonicFormat;           /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgAmbisonicChnOrder;         /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgAmbisonicNorm;             /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgMicType;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgMicConfig;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgMicDistance;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgRecordingLoc;              /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIsDesigned;                /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgRecEngineer;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgRecStudio;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgImpulseLocation;           /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgCategory;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgSubCategory;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgCatId;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgUserCategory;              /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgUserData;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgVendorCategory;            /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgFxName;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgLibrary;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgCreatorId;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgSourceId;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgRmsPower;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgLoudness;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgLoudnessRange;             /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgMaxPeak;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgSpecDensity;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgZeroCrossRate;             /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgPapr;                      /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgText;                      /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgEfforts;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgEffortType;                /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgProjection;                /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgLanguage;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgTimingRestriction;         /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgCharacterName;             /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgCharacterGender;           /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgCharacterAge;              /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgCharacterRole;             /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgActorName;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgActorGender;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgDirector;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgDirection;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgFxUsed;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgUsageRights;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIsUnion;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgAccent;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgEmotion;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgComposor;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgArtist;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgSongTitle;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgGenre;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgSubGenre;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgProducer;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgMusicSup;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgInstrument;                /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgMusicPublisher;            /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgRightsOwner;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIsSource;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIsLoop;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIntensity;                 /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIsFinal;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgOrderRef;                  /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIsOst;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIsCinematic;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIsLicensed;                /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIsDiegetic;                /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgMusicVersion;              /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgIsrcId;                    /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgTempo;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgTimeSig;                   /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgInKey;                     /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgBillingCode;               /**< Metadata property name used in ASWG/iXML chunks. */
    static const char* const aswgVersion;                   /**< Metadata property name used in ASWG/iXML chunks. */

    //==============================================================================
    /** Metadata property name used when reading an ISRC code from an AXML chunk. */
    [[deprecated ("This string is identical to riffInfoSource, making it impossible to differentiate between the two")]]
    static const char* const ISRC;

    /** Metadata property name used when reading and writing ISRC codes to/from AXML chunks. */
    static const char* const internationalStandardRecordingCode;

    /** Metadata property name used when reading a WAV file with a Tracktion chunk. */
    static const char* const tracktionLoopInfo;

    //==============================================================================
    Array<int> getPossibleSampleRates() override;
    Array<int> getPossibleBitDepths() override;
    bool canDoStereo() override;
    bool canDoMono() override;
    bool isChannelLayoutSupported (const AudioChannelSet& channelSet) override;

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                        bool deleteStreamIfOpeningFails) override;

    MemoryMappedAudioFormatReader* createMemoryMappedReader (const File&)      override;
    MemoryMappedAudioFormatReader* createMemoryMappedReader (FileInputStream*) override;

    std::unique_ptr<AudioFormatWriter> createWriterFor (std::unique_ptr<OutputStream>& streamToWriteTo,
                                                        const AudioFormatWriterOptions& options) override;

    using AudioFormat::createWriterFor;

    //==============================================================================
    /** Utility function to replace the metadata in a wav file with a new set of values.

        If possible, this cheats by overwriting just the metadata region of the file, rather
        than by copying the whole file again.
    */
    bool replaceMetadataInFile (const File& wavFile, const StringPairArray& newMetadata);


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavAudioFormat)
};

} // namespace juce
