/******************************************************************************
**
** Copyright (c) 2017, Swedish National Road and Transport Research Institute
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** - Redistributions of source code must retain the above copyright notice,
**   this list of conditions and the following disclaimer.
** - Redistributions in binary form must reproduce the above copyright
**   notice, this list of conditions and the following disclaimer in the
**   documentation and/or other materials provided with the distribution.
** - Neither the name of the Swedish National Road and Transport Research
**   Institute nor the names of its contributors may be used to endorse or
**   promote products derived from this software without specific prior
**   written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
******************************************************************************/

#include "commandlineparser.h"

#include <QtCore/QStringList>


CommandLineOption::CommandLineOption(const QString& name, const QString& description) :
  m_name(name),
  m_description(description)
{
}

CommandLineOption::CommandLineOption(const QString& name, const QString& description,
                                     const QString& value_name, const QString& default_value) :
  m_name(name),
  m_description(description),
  m_value_name(value_name),
  m_default_value(default_value)
{
}


void CommandLineParser::addOption(const CommandLineOption& option) {
  m_options.insert(option.getName(), option);
}

bool CommandLineParser::parse(const int argc, char** argv) {
  QStringList args;

  for (int i = 0; i < argc; ++i) {
    args.append(QString::fromLatin1(argv[i]));
  }

  return parse(args);
}

bool CommandLineParser::parse(const QStringList& arguments) {
  QStringList args = arguments;
  args.removeFirst(); // remove name of executable

  while (!args.isEmpty()) {
    const QString option_name = args.takeFirst();

    if (m_options.contains(option_name)) {
      QVariantList& values = m_values[option_name];

      // read values until next option found
      while (!args.isEmpty() && !args.first().startsWith("-")) {
        values.append(args.takeFirst());
      }
    }
    else {
      // unknown option
      m_parse_error = QString("Unknown option '%1'").arg(option_name);
      return false;
    }
  }

  return true;
}

bool CommandLineParser::isSet(const QString& option_name) const {
  return m_values.contains(option_name);
}

QVariantList CommandLineParser::getValuesAsVariants(const QString& option_name) const {
  if (m_values.contains(option_name)) {
    // return values
    return m_values[option_name];
  }
  else if (m_options.contains(option_name)) {
    // return default value
    return (QVariantList() << m_options[option_name].getDefaultValue());
  }
  else {
    return QVariantList();
  }
}

QString CommandLineParser::getHelpText() const {
  QStringList help_text_lines;

  foreach (const CommandLineOption& option, m_options) {
    help_text_lines.append(QString("%1 %2 %3 %4").arg(option.getName(), option.getValueName(),
                                                      option.getDescription(), option.getDefaultValue()));
  }


  return help_text_lines.join(QChar('\n'));
}
