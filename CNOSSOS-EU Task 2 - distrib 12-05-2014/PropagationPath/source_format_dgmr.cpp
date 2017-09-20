/*
 * Parse an external file containing the source description
 *
 * see documentation of the CNOSSOS-EU source modules for details on the semantics of 
 * external files containing sound power 
 */
static bool ParseExternalSource (XMLNode* node, SourceExt& source)
{
	/*
	 * root node must be "CNOSSOS_SourcePower"
	 */
	if (!checkTagName (node, "CNOSSOS_SourcePower"))
	{
		signal_error (XMLUnexpectedTag(node)) ;
		return false ;
	}
	/*
	 * root accepts a single child entity, tagged "SourcePower"
	 *
	 * the "SourcePower" entity, in turn, contains the following child items: SourceHeight, SourceType,
	 * RadiationType, SpectrumWeighting and Spectrum. All items are mandatory and must appear in the 
	 * specified order.
	 *
	 * note that, as the identifiers are encoded as text values, no white space is allowed between the
	 * values and the opening/closing tags !
	 */
	node = node->GetFirstChild() ;
	if (!checkTagName (node, "SourcePower"))
	{
		signal_error (XMLUnexpectedTag(node)) ;
		return false ;
	}
	/*
	 * get the height of the source
	 */
	node = node->GetFirstChild () ;
	if (!ParseValue (node, "SourceHeight", source.h))
	{
		signal_error (XMLUnexpectedTag(node)) ;
		return false ;
	}
	/*
	 * get the sound power units
	 */
	node = node->GetNextEntity() ;
	if (!checkTagName (node, "SourceType"))
	{
		signal_error (XMLUnexpectedTag(node)) ;
		return false ;
	}
	const char* sourceType = node->GetText() ;
	if (strcmp(sourceType, "point") == 0) source.spectrumType = SpectrumType::PointSource ;
	else if (strcmp(sourceType, "line") == 0)  source.spectrumType = SpectrumType::LineSource ;
	else if (strcmp(sourceType, "area") == 0)  source.spectrumType = SpectrumType::AreaSource ;
	else
	{
		signal_error (XMLParseError ("Invalid parameter value", node)) ;
	}
	/*
	 * get the radiation type
	 */
	node = node->GetNextEntity() ;
	if (!checkTagName (node, "RadiationType"))
	{
		signal_error (XMLUnexpectedTag(node)) ;
		return false ;
	}
	const char* radiationType = node->GetText() ;
	if (strcmp(radiationType, "freefield") == 0)     source.measurementType = MeasurementType::FreeField ;
	else if (strcmp(radiationType, "hemispheric") == 0) source.measurementType = MeasurementType::HemiSpherical ;
	else
	{
		signal_error (XMLParseError ("Invalid parameter value", node)) ;
	}
	/*
	 * get the frequency weighting 
	 */
	node = node->GetNextEntity() ;
	if (!checkTagName (node, "SpectrumWeighting"))
	{
		signal_error (XMLUnexpectedTag(node)) ;
		return false ;
	}
	const char* frequencyWeighting = node->GetText();
	if (strcmp(frequencyWeighting, "lin") == 0) source.frequencyWeighting = FrequencyWeighting::dBLIN ;
	else if (strcmp(frequencyWeighting, "dBA") == 0) source.frequencyWeighting = FrequencyWeighting::dBA ;
	else
	{
		signal_error (XMLParseError ("Invalid parameter value", node)) ;
	}
	/*
	 * get the spectrum
	 */
	node = node->GetNextEntity() ;
	if (!ParseSpectrum(node, "Spectrum", source.soundPower))
	{
		signal_error (XMLUnexpectedTag(node)) ;
		return false ;
	}
	/*
	 * no extra child entities allowed
	 */
	node = node->GetNextEntity() ;
	if (node != 0)
	{
		signal_error (XMLUnexpectedTag(node)) ;
		return false ;
	}

	return true ;
}
