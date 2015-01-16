#pragma once

#include <QDomDocument>
#include <QXmlQuery>

namespace shadertoy {

  static const char * CHANNEL_REGEX = "(\\w+)(\\d{2})";
  static const char * XML_ROOT_NAME = "shadertoy";
  static const char * XML_FRAGMENT_SOURCE = "fragmentSource";
  static const char * XML_CHANNEL = "channel";
  static const char * XML_CHANNEL_ATTR_ID = "id";
  static const char * XML_CHANNEL_ATTR_SOURCE = "source";
  static const char * XML_CHANNEL_ATTR_TYPE = "type";

  ChannelInputType channelTypeFromString(const QString & channelTypeStr) {
    ChannelInputType channelType = ChannelInputType::TEXTURE;
    if (channelTypeStr == "tex") {
      channelType = ChannelInputType::TEXTURE;
    } else if (channelTypeStr == "cube") {
      channelType = ChannelInputType::CUBEMAP;
    }
    return channelType;
  }

  Shader loadShaderXml(QIODevice & ioDevice) {
    QDomDocument dom;
    Shader result;
    dom.setContent(&ioDevice);

    auto children = dom.documentElement().childNodes();
    for (int i = 0; i < children.count(); ++i) {
      auto child = children.at(i);
      if (child.nodeName() == "url") {
        result.url = child.firstChild().nodeValue().toLocal8Bit();
      } if (child.nodeName() == XML_FRAGMENT_SOURCE) {
        result.fragmentSource = child.firstChild().nodeValue().toLocal8Bit();
      } else if (child.nodeName() == XML_CHANNEL) {
        auto attributes = child.attributes();
        int channelIndex = -1;
        QString source;
        if (attributes.contains(XML_CHANNEL_ATTR_ID)) {
          channelIndex = attributes.namedItem(XML_CHANNEL_ATTR_ID).nodeValue().toInt();
        }

        if (channelIndex < 0 || channelIndex >= shadertoy::MAX_CHANNELS) {
          continue;
        }


        // Compatibility mode
        if (attributes.contains(XML_CHANNEL_ATTR_SOURCE)) {
          source = attributes.namedItem(XML_CHANNEL_ATTR_SOURCE).nodeValue();
          QRegExp re(CHANNEL_REGEX);
          if (!re.exactMatch(source)) {
            continue;
          }
          result.channelTypes[channelIndex] = channelTypeFromString(re.cap(1));
          result.channelTextures[channelIndex] = ("preset://" + re.cap(1) + "/" + re.cap(2)).toLocal8Bit();
          continue;
        }

        if (attributes.contains(XML_CHANNEL_ATTR_TYPE)) {
          result.channelTypes[channelIndex] = channelTypeFromString(attributes.namedItem(XML_CHANNEL_ATTR_SOURCE).nodeValue());
          result.channelTextures[channelIndex] = child.firstChild().nodeValue().toLocal8Bit();
        }
      }
    }
    return result;
  }

  Shader loadShaderXml(Resource res) {
    QByteArray presetData = oria::qt::toByteArray(res);
    QBuffer buffer(&presetData);
    return loadShaderXml(buffer);
  }

  Shader loadShaderXml(const QString & fileName) {
    QFile file(fileName);
    return loadShaderXml(file);
  }

  // FIXME no error handling.  
  QDomDocument writeShaderXml(const Shader & shader) {
    QDomDocument result;
    QDomElement root = result.createElement(XML_ROOT_NAME);
    result.appendChild(root);

    for (int i = 0; i < MAX_CHANNELS; ++i) {
      if (!shader.channelTextures[i].empty()) {
        QDomElement channelElement = result.createElement(XML_CHANNEL);
        channelElement.setAttribute(XML_CHANNEL_ATTR_ID, i);
        channelElement.setAttribute(XML_CHANNEL_ATTR_TYPE, shader.channelTypes[i] == ChannelInputType::CUBEMAP ? "cube" : "tex");
        channelElement.appendChild(result.createTextNode(shader.channelTextures[i].c_str()));
        root.appendChild(channelElement);
      }
    }
    root.appendChild(result.createElement(XML_FRAGMENT_SOURCE)).
      appendChild(result.createCDATASection(shader.fragmentSource.c_str()));

    return result;
  }


  // FIXME no error handling.  
  void saveShaderXml(const QString & name, const Shader & shader) {
    QDomDocument doc = writeShaderXml(shader);
    QFile file(name);
    file.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text);
    file.write(doc.toByteArray());
    file.close();
  }
}