#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/vorbisfile.h>
#include <taglib/tfilestream.h>
#include "decoder_vorbis.h"
#include "vorbismetadatamodel.h"
#include "decodervorbisfactory.h"

bool DecoderVorbisFactory::canDecode(QIODevice *input) const
{
    char buf[36];
    if(input->peek(buf, 36) == 36 && !memcmp(buf, "OggS", 4)
            && !memcmp(buf + 29, "vorbis", 6))
        return true;

    return false;
}

DecoderProperties DecoderVorbisFactory::properties() const
{
    DecoderProperties properties;
    properties.name = tr("Ogg Vorbis Plugin");
    properties.shortName = "vorbis";
    properties.filters << "*.ogg";
    properties.description = tr("Ogg Vorbis Files");
    properties.contentTypes << "application/ogg" << "audio/x-vorbis+ogg";
    return properties;
}

Decoder *DecoderVorbisFactory::create(const QString &path, QIODevice *input)
{
    Q_UNUSED(path);
    return new DecoderVorbis(input);
}

MetaDataModel* DecoderVorbisFactory::createMetaDataModel(const QString &path, bool readOnly)
{
    return new VorbisMetaDataModel(path, readOnly);
}

QList<TrackInfo*> DecoderVorbisFactory::createPlayList(const QString &path, TrackInfo::Parts parts, QStringList *)
{
    TrackInfo *info = new TrackInfo(path);

    if(parts == TrackInfo::Parts())
    {
        return QList<TrackInfo*>() << info;
    }

    TagLib::FileStream stream(QStringToFileName(path), true);
    TagLib::Ogg::Vorbis::File fileRef(&stream);

    if((parts & TrackInfo::MetaData) && fileRef.tag() && !fileRef.tag()->isEmpty())
    {
        TagLib::Ogg::XiphComment *tag = fileRef.tag();
        info->setValue(Qmmp::ALBUM, TStringToQString(tag->album()));
        info->setValue(Qmmp::ARTIST, TStringToQString(tag->artist()));
        info->setValue(Qmmp::COMMENT, TStringToQString(tag->comment()));
        info->setValue(Qmmp::GENRE, TStringToQString(tag->genre()));
        info->setValue(Qmmp::TITLE, TStringToQString(tag->title()));
        info->setValue(Qmmp::YEAR, tag->year());
        info->setValue(Qmmp::TRACK, tag->track());

        TagLib::Ogg::FieldListMap items = tag->fieldListMap();
        if(!items["ALBUMARTIST"].isEmpty())
            info->setValue(Qmmp::ALBUMARTIST, TStringToQString(items["ALBUMARTIST"].toString()));
         if(!items["COMPOSER"].isEmpty())
            info->setValue(Qmmp::COMPOSER, TStringToQString(items["COMPOSER"].toString()));
        if(!items["DISCNUMBER"].isEmpty())
            info->setValue(Qmmp::DISCNUMBER, TStringToQString(items["DISCNUMBER"].toString()));
    }

    TagLib::Vorbis::Properties *ap = fileRef.audioProperties();
    if((parts & TrackInfo::Properties) && ap)
    {
        info->setValue(Qmmp::BITRATE, ap->bitrate());
        info->setValue(Qmmp::SAMPLERATE, ap->sampleRate());
        info->setValue(Qmmp::CHANNELS, ap->channels());
        info->setValue(Qmmp::BITS_PER_SAMPLE, 32);  //float
        info->setValue(Qmmp::FORMAT_NAME, "Ogg Vorbis");
        info->setDuration(ap->lengthInMilliseconds());
    }

    if((parts & TrackInfo::ReplayGainInfo) && fileRef.tag() && !fileRef.tag()->isEmpty())
    {
        TagLib::Ogg::XiphComment *tag = fileRef.tag();
        TagLib::Ogg::FieldListMap items = tag->fieldListMap();
        if(items.contains("REPLAYGAIN_TRACK_GAIN"))
            info->setValue(Qmmp::REPLAYGAIN_TRACK_GAIN, TStringToQString(items["REPLAYGAIN_TRACK_GAIN"].front()));
        if(items.contains("REPLAYGAIN_TRACK_PEAK"))
            info->setValue(Qmmp::REPLAYGAIN_TRACK_PEAK, TStringToQString(items["REPLAYGAIN_TRACK_PEAK"].front()));
        if(items.contains("REPLAYGAIN_ALBUM_GAIN"))
            info->setValue(Qmmp::REPLAYGAIN_ALBUM_GAIN, TStringToQString(items["REPLAYGAIN_ALBUM_GAIN"].front()));
        if(items.contains("REPLAYGAIN_ALBUM_PEAK"))
            info->setValue(Qmmp::REPLAYGAIN_ALBUM_PEAK, TStringToQString(items["REPLAYGAIN_ALBUM_PEAK"].front()));
    }

    return QList<TrackInfo*>() << info;
}

#ifndef QMMP_GREATER_NEW
#include <QtPlugin>
Q_EXPORT_PLUGIN2(vorbis, DecoderVorbisFactory)
#endif