// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2013.
//
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS.
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// --------------------------------------------------------------------------
// $Maintainer: Chris Bielow $
// $Authors: Nico Pfeifer, Chris Bielow $
// --------------------------------------------------------------------------

#include <OpenMS/FORMAT/IdXMLFile.h>
#include <OpenMS/FILTERING/ID/IDFilter.h>
#include <OpenMS/FORMAT/FASTAFile.h>
#include <OpenMS/METADATA/PeptideIdentification.h>
#include <OpenMS/SYSTEM/File.h>
#include <OpenMS/APPLICATIONS/TOPPBase.h>

#include <limits>
#include <cmath>
#include <set>

using namespace OpenMS;
using namespace std;

//-------------------------------------------------------------
//Doxygen docu
//-------------------------------------------------------------

/**
 @page TOPP_IDFilter IDFilter

 @brief Filters protein identification engine results by different criteria.
<CENTER>
 <table>
  <tr>
   <td ALIGN = "center" BGCOLOR="#EBEBEB"> potential predecessor tools </td>
   <td VALIGN="middle" ROWSPAN=5> \f$ \longrightarrow \f$ IDFilter \f$ \longrightarrow \f$</td>
   <td ALIGN = "center" BGCOLOR="#EBEBEB"> potential successor tools </td>
  </tr>
  <tr>
   <td VALIGN="middle" ALIGN = "center" ROWSPAN=1> @ref TOPP_MascotAdapter (or other ID engines) </td>
   <td VALIGN="middle" ALIGN = "center" ROWSPAN=1> @ref TOPP_PeptideIndexer </td>
  </tr>
  <tr>
   <td VALIGN="middle" ALIGN = "center" ROWSPAN=1> @ref TOPP_IDFileConverter </td>
   <td VALIGN="middle" ALIGN = "center" ROWSPAN=1> @ref TOPP_ProteinInference </td>
  </tr>
  <tr>
   <td VALIGN="middle" ALIGN = "center" ROWSPAN=1> @ref TOPP_FalseDiscoveryRate </td>
   <td VALIGN="middle" ALIGN = "center" ROWSPAN=2> @ref TOPP_IDMapper </td>
  </tr>
  <tr>
   <td VALIGN="middle" ALIGN = "center" ROWSPAN=1> @ref TOPP_ConsensusID </td>
  </tr>
 </table>
</CENTER>

 This tool is used to filter the identifications found by
 a peptide/protein identification tool like Mascot. Different filters can be applied:

 To enable any of the filters, just change their default value.
 All active filters will be applied in order.

 <ul>

  <li>
   <b>precursor:rt</b>:<br> Precursor RT range for the peptide identification to be kept.
  </li>
  <li>
   <b>precursor:mz</b>:<br> Precursor m/z range for the peptide identification to be kept.
  </li>
  <li>
   <b>score:pep</b>:<br> The score a peptide hit should have to be kept.
  </li>
  <li>
   <b>score:prot</b>:<br> The score a protein hit should have to be kept.
  </li>
  <li>
   <b>thresh:pep</b>:<br> The fraction of the significance threshold that should
   be reached by a peptide hit to be kept. If for example a peptide
   has score 30 and the significance threshold is 40, the
   peptide will only be kept by the filter if the significance
   threshold fraction is set to 0.75 or lower.
  </li>
  <li>
   <b>thresh:prot</b>:<br> This parameter
   behaves in the same way as the peptide significance threshold
   fraction parameter. The only difference is that it is used
   to filter protein hits.
  </li>
  <li>
   <b>whitelist:proteins</b>:<br> If you know which proteins
   are in the measured sample you can specify a FASTA file
   which contains the protein sequences of those proteins. All
   peptides which are not a substring of a protein contained
   in the sequences file will be filtered out. The filtering is based on the
      protein identifiers attached to the peptide hits. Protein Hits not matching
      any FASTA protein are also removed.<br>
      If you want filtering using the sequence alone, then use the flag @em WhiteList:by_seq_only.
  </li>
  <li>
   <b>blacklist:peptides</b>:<br> For this option you specify an idXML file.
   All peptides that are present in both files (in-file and exclusion peptides
   file) will be dropped. Protein Hits are not affected.
  </li>
  <li><b>rt</b>:<br> To filter identifications according to their
   predicted retention times you have to set 'rt:p_value' and/or 'rt:p_value_1st_dim' larger than 0, depending which RT
      dimension you want to filter.
   This filter can only be applied to idXML files produced by @ref TOPP_RTPredict.
  </li>
  <li>
   <b>best:n_peptide_hits</b>:<br> Only the best n peptide hits of a spectrum are kept. If two hits have the same score, their order is random.
  </li>
  <li>
   <b>best:n_protein_hits</b>:<br> Only the best n protein hits of a spectrum are kept. If two hits have the same score, their order is random.
  </li>
  <li>
   <b>best:strict</b>:<br> Only the best hit of a spectrum is kept.
   If there is more than one hit for a spectrum with the maximum score, then
   none of the hits will be kept. This is similar to n_peptide_hits=1, but if there are two or more highest scoring hits, none are kept.
  </li>
 </ul>

 <B>The command line parameters of this tool are:</B>
 @verbinclude TOPP_IDFilter.cli
    <B>INI file documentation of this tool:</B>
    @htmlinclude TOPP_IDFilter.html
*/

// We do not want this class to show up in the docu:
/// @cond TOPPCLASSES


class TOPPIDFilter :
  public TOPPBase
{
public:
  TOPPIDFilter() :
    TOPPBase("IDFilter", "Filters results from protein or peptide identification engines based on different criteria.")
  {

  }

protected:

  void registerOptionsAndFlags_()
  {
    registerInputFile_("in", "<file>", "", "input file ");
    setValidFormats_("in", StringList::create("idXML"));
    registerOutputFile_("out", "<file>", "", "output file ");
    setValidFormats_("out", StringList::create("idXML"));

    registerTOPPSubsection_("precursor", "Filtering by precursor RT or m/z");
    registerStringOption_("precursor:rt", "[min]:[max]", ":", "Retention time range to extract.", false);
    registerStringOption_("precursor:mz", "[min]:[max]", ":", "Mass-to-charge range to extract.", false);
    registerFlag_("precursor:allow_missing", "When filtering by precursor RT or m/z, keep peptide IDs with missing precursor information ('RT'/'MZ' meta values)?");

    registerTOPPSubsection_("score", "Filtering by peptide/protein score. To enable any of the filters below, just change their default value. All active filters will be applied in order.");
    registerDoubleOption_("score:pep", "<score>", 0, "The score which should be reached by a peptide hit to be kept. The score is dependent on the most recent(!) preprocessing - it could be Mascot scores (if a MascotAdapter was applied before), or an FDR (if FalseDiscoveryRate was applied before), etc.", false);
    registerDoubleOption_("score:prot", "<score>", 0, "The score which should be reached by a protein hit to be kept.", false);
    registerTOPPSubsection_("thresh", "Filtering by significance threshold");
    registerDoubleOption_("thresh:pep", "<fraction>", 0.0, "Keep a peptide hit only if its score is above this fraction of the peptide significance threshold.", false);
    registerDoubleOption_("thresh:prot", "<fraction>", 0.0, "Keep a protein hit only if its score is above this fraction of the protein significance threshold.", false);

    registerTOPPSubsection_("whitelist", "Filtering by whitelisting (only instances also present in a whitelist file can pass)");
    registerInputFile_("whitelist:proteins", "<file>", "", "filename of a FASTA file containing protein sequences.\n"
                                                           "All peptides that are not a substring of a sequence in this file are removed\n"
                                                           "All proteins whose accession is not present in this file are removed.", false);
    setValidFormats_("whitelist:proteins", StringList::create("fasta"));
    registerFlag_("whitelist:by_seq_only", "Match peptides with FASTA file by sequence instead of accession and disable protein filtering.");

    registerTOPPSubsection_("blacklist", "Filtering by blacklisting (only instances not present in a blacklist file can pass)");
    registerInputFile_("blacklist:peptides", "<file>", "", "Peptides having the same sequence as any peptide in this file will be filtered out\n", false);
    setValidFormats_("blacklist:peptides", StringList::create("idXML"));

    registerTOPPSubsection_("rt", "Filtering by RT predicted by 'RTPredict'");
    registerDoubleOption_("rt:p_value", "<float>", 0.0, "Retention time filtering by the p-value predicted by RTPredict.", false);
    registerDoubleOption_("rt:p_value_1st_dim", "<float>", 0.0, "Retention time filtering by the p-value predicted by RTPredict for first dimension.", false);
    setMinFloat_("rt:p_value", 0);
    setMaxFloat_("rt:p_value", 1);
    setMinFloat_("rt:p_value_1st_dim", 0);
    setMaxFloat_("rt:p_value_1st_dim", 1);

    registerTOPPSubsection_("mz", "Filtering by mz");
    registerDoubleOption_("mz:error", "<float>", -1, "Filtering by deviation to theoretical mass (disabled for negative values).", false);
    registerStringOption_("mz:unit", "<String>", "ppm", "Absolute or relativ error.", false);
    setValidStrings_("mz:unit", StringList::create("Da,ppm"));

    registerTOPPSubsection_("best", "Filtering best hits per spectrum (for peptides) or from proteins");
    registerIntOption_("best:n_peptide_hits", "<integer>", 0, "Keep only the 'n' highest scoring peptide hits per spectrum (for n>0).", false);
    setMinInt_("best:n_peptide_hits", 0);
    registerIntOption_("best:n_protein_hits", "<integer>", 0, "Keep only the 'n' highest scoring protein hits (for n>0).", false);
    setMinInt_("best:n_protein_hits", 0);
    registerFlag_("best:strict", "Keep only the highest scoring peptide hit.\n"
                                 "Similar to n_peptide_hits=1, but if there are two or more highest scoring hits, none are kept.");
    registerStringOption_("best:n_to_m_peptide_hits", "[min]:[max]", ":", "peptide hit rank range to extracts", false, true);
    registerIntOption_("min_length", "<integer>", 0, "Keep only peptide hits with a length greater or equal this value. Value 0 will have no filter effect.", false);
    setMinInt_("min_length", 0);
    registerIntOption_("max_length", "<integer>", 0, "Keep only peptide hits with a length less or equal this value. Value 0 will have no filter effect. Value is overridden by min_length, i.e. if max_length < min_length, max_length will be ignored.", false);
    setMaxInt_("max_length", 0);
    registerIntOption_("min_charge", "<integer>", 1, "Keep only peptide hits for tandem spectra with charge greater or equal this value.", false);
    setMinInt_("min_charge", 1);
    registerFlag_("var_mods", "Keep only peptide hits with variable modifications (fixed modifications from SearchParameters will be ignored).", false);

    registerFlag_("unique", "If a peptide hit occurs more than once per PSM, only one instance is kept.");
    registerFlag_("unique_per_protein", "Only peptides matching exactly one protein are kept. Remember that isoforms count as different proteins!");
    registerFlag_("keep_unreferenced_protein_hits", "Proteins not referenced by a peptide are retained in the idXML.");

    //setSectionDescription("RT", "Filters peptides using meta-data annotated by RT predict. The criterion is always the p-value (for having a deviation between observed and predicted RT equal or bigger than allowed).");

  }

  ExitCodes main_(int, const char**)
  {

    //-------------------------------------------------------------
    // variables
    //-------------------------------------------------------------

    IDFilter filter;
    IdXMLFile idXML_file;
    vector<ProteinIdentification> protein_identifications;
    vector<PeptideIdentification> identifications;
    vector<PeptideIdentification> identifications_exclusion;
    vector<PeptideIdentification> filtered_peptide_identifications;
    vector<ProteinIdentification> filtered_protein_identifications;
    PeptideIdentification filtered_identification;
    ProteinIdentification filtered_protein_identification;
    vector<FASTAFile::FASTAEntry> sequences;
    set<String> exclusion_peptides;


    //-------------------------------------------------------------
    // parsing parameters
    //-------------------------------------------------------------

    String inputfile_name = getStringOption_("in");
    String outputfile_name = getStringOption_("out");

    DoubleReal peptide_significance_threshold_fraction = getDoubleOption_("thresh:pep");
    DoubleReal protein_significance_threshold_fraction = getDoubleOption_("thresh:prot");
    DoubleReal peptide_threshold_score = getDoubleOption_("score:pep");
    DoubleReal protein_threshold_score = getDoubleOption_("score:prot");

    Int best_n_peptide_hits = getIntOption_("best:n_peptide_hits");
    Int best_n_protein_hits = getIntOption_("best:n_protein_hits");

    Int best_n_to_m_peptide_hits_n = 0;
    Int best_n_to_m_peptide_hits_m = numeric_limits<Int>::max();

    const double double_max = numeric_limits<double>::max();
    DoubleReal rt_low, rt_high, mz_low, mz_high;
    rt_high = mz_high = double_max;
    rt_low = mz_low = -double_max;

    //convert bounds to numbers
    try
    {
      parseRange_(getStringOption_("best:n_to_m_peptide_hits"), best_n_to_m_peptide_hits_n, best_n_to_m_peptide_hits_m);
      parseRange_(getStringOption_("precursor:rt"), rt_low, rt_high);
      parseRange_(getStringOption_("precursor:mz"), mz_low, mz_high);
    }
    catch (Exception::ConversionError& ce)
    {
      writeLog_(String("Invalid boundary given: ") + ce.what() + ". Aborting!");
      printUsage_();
      return ILLEGAL_PARAMETERS;
    }

    bool precursor_missing = getFlag_("precursor:allow_missing");
    bool best_strict = getFlag_("best:strict");
    UInt min_length = getIntOption_("min_length");
    UInt max_length = getIntOption_("max_length");
    UInt min_charge = getIntOption_("min_charge");

    bool var_mods = getFlag_("var_mods");

    String sequences_file_name = getStringOption_("whitelist:proteins").trim();
    bool no_protein_identifiers = getFlag_("whitelist:by_seq_only");

    String exclusion_peptides_file_name = getStringOption_("blacklist:peptides").trim();

    DoubleReal pv_rt_filtering = getDoubleOption_("rt:p_value");
    DoubleReal pv_rt_filtering_1st_dim = getDoubleOption_("rt:p_value_1st_dim");

    bool unique = getFlag_("unique");
    bool unique_per_protein = getFlag_("unique_per_protein");

    bool keep_unreferenced_protein_hits = getFlag_("keep_unreferenced_protein_hits");

    DoubleReal mz_error = getDoubleOption_("mz:error");
    bool mz_error_filtering = (mz_error < 0) ? false : true;
    bool mz_error_unit_ppm = (getStringOption_("mz:unit") == "ppm") ? true : false;

    //-------------------------------------------------------------
    // reading input
    //-------------------------------------------------------------


    if (sequences_file_name != "")
    {
      FASTAFile().load(sequences_file_name, sequences);
    }

    // preprocessing
    if (exclusion_peptides_file_name  != "")
    {
      String document_id;
      idXML_file.load(exclusion_peptides_file_name, protein_identifications, identifications_exclusion, document_id);
      for (Size i = 0; i < identifications_exclusion.size(); i++)
      {
        for (vector<PeptideHit>::const_iterator it = identifications_exclusion[i].getHits().begin();
             it != identifications_exclusion[i].getHits().end();
             ++it)
        {
          exclusion_peptides.insert(it->getSequence().toString());
        }
      }
    }
    String document_id;
    idXML_file.load(inputfile_name, protein_identifications, identifications, document_id);

    //-------------------------------------------------------------
    // calculations
    //-------------------------------------------------------------

    std::set<String> applied_filters;

    // Filtering peptide identification according to set criteria
    for (Size i = 0; i < identifications.size(); i++)
    {
      if ((rt_high < double_max) || (rt_low > -double_max))
      {
        applied_filters.insert("Filtering by precursor RT ...\n");
        if (!filter.filterIdentificationsByMetaValueRange(identifications[i], "RT", rt_low, rt_high, precursor_missing)) continue; // don't keep this peptide ID
      }

      if ((mz_high < double_max) || (mz_low > -double_max))
      {
        applied_filters.insert("Filtering by precursor m/z ...\n");
        if (!filter.filterIdentificationsByMetaValueRange(identifications[i], "MZ", mz_low, mz_high, precursor_missing)) continue; // don't keep this peptide ID
      }

      if (unique_per_protein)
      {
        applied_filters.insert("Filtering unique per proteins ...\n");
        vector<PeptideHit> hits;
        for (vector<PeptideHit>::const_iterator it = identifications[i].getHits().begin(); it != identifications[i].getHits().end(); ++it)
        {
          if (!it->metaValueExists("protein_references"))
          {
            writeLog_("IDFilter: Warning, filtering with 'unique_per_protein' can only be done after indexing the file with 'PeptideIndexer' first.");
          }
          if (it->metaValueExists("protein_references") && (String)it->getMetaValue("protein_references") == "unique")
          {
            hits.push_back(*it);
          }
        }
        identifications[i].setHits(hits);
      }

      if (fabs(peptide_significance_threshold_fraction - 0) < 0.00001)
      {
        filtered_identification = identifications[i];
      }
      else
      {
        filter.filterIdentificationsByThreshold(identifications[i], peptide_significance_threshold_fraction, filtered_identification);
        applied_filters.insert("Filtering by peptide significance threshold ...\n");
      }
      if (sequences_file_name != "")
      {
        applied_filters.insert("Filtering by peptide sequence whitelisting ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByProteins(temp_identification, sequences, filtered_identification, no_protein_identifiers);
      }

      if (pv_rt_filtering > 0)
      {
        applied_filters.insert("Filtering by RT p-value ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByRTPValues(temp_identification, filtered_identification, pv_rt_filtering);
      }

      if (pv_rt_filtering_1st_dim > 0)
      {
        applied_filters.insert("Filtering by RT p-value (first dimension) ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByRTFirstDimPValues(temp_identification, filtered_identification, pv_rt_filtering_1st_dim);
      }

      if (exclusion_peptides_file_name != "")
      {
        applied_filters.insert("Filtering by exclusion peptide blacklisting ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByExclusionPeptides(temp_identification, exclusion_peptides, filtered_identification);
      }

      if (unique)
      {
        applied_filters.insert("Filtering by unique peptide ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsUnique(temp_identification, filtered_identification);
      }

      if (best_strict)
      {
        applied_filters.insert("Filtering by best hits only ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByBestHits(temp_identification, filtered_identification, true);
      }

      if (min_length > 0 || max_length > 0)
      {
        applied_filters.insert(String("Filtering peptide length [lower bound, upper bound]") +  min_length + " , " + max_length + "...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByLength(temp_identification,
                                             filtered_identification,
                                             min_length,
                                             max_length);
      }

      if (var_mods)
      {
        // determine fixed modifications to distinguish variable modifications
        vector<ProteinIdentification::SearchParameters> search_params;
        vector<String> fixed_modifications;
        for (vector<ProteinIdentification>::const_iterator it = protein_identifications.begin(); it != protein_identifications.end(); ++it)
        {
          if (find(search_params.begin(), search_params.end(), it->getSearchParameters()) == search_params.end())
          {
            search_params.push_back(it->getSearchParameters());
          }
        }
        for (Size i = 0; i != search_params.size(); ++i)
        {
          for (Size j = 0; j != search_params[i].fixed_modifications.size(); ++j)
          {
            fixed_modifications.push_back(search_params[i].fixed_modifications[j]);
          }
        }

        applied_filters.insert(String("Filtering for variable modifications") +  "...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByVariableModifications(temp_identification, fixed_modifications, filtered_identification);
      }

      if (peptide_threshold_score != 0)
      {
        applied_filters.insert(String("Filtering by peptide score < (or >) ") + peptide_threshold_score + " ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByScore(temp_identification, peptide_threshold_score, filtered_identification);
      }

      if (min_charge > 1)
      {
        applied_filters.insert(String("Filtering by charge > ") + min_charge + " ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByCharge(temp_identification, min_charge, filtered_identification);
      }

      if (best_n_peptide_hits != 0)
      {
        applied_filters.insert("Filtering by best n peptide hits ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByBestNHits(temp_identification, best_n_peptide_hits, filtered_identification);
      }

      if (best_n_to_m_peptide_hits_m != numeric_limits<Int>::max() || best_n_to_m_peptide_hits_n != 0)
      {
        applied_filters.insert("Filtering by best n to m peptide hits ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByBestNToMHits(temp_identification, best_n_to_m_peptide_hits_n, best_n_to_m_peptide_hits_m, filtered_identification);
      }

      if (mz_error_filtering)
      {
        applied_filters.insert("Filtering by mass error ...\n");
        PeptideIdentification temp_identification = filtered_identification;
        filter.filterIdentificationsByMzError(temp_identification, mz_error, mz_error_unit_ppm, filtered_identification);
      }

      if (!filtered_identification.getHits().empty())
      {
        filtered_identification.setMetaValue("RT", identifications[i].getMetaValue("RT"));
        filtered_identification.setMetaValue("MZ", identifications[i].getMetaValue("MZ"));
        filtered_peptide_identifications.push_back(filtered_identification);
      }

    }

    // Filtering protein identifications according to set criteria
    for (Size i = 0; i < protein_identifications.size(); i++)
    {
      if (!protein_identifications[i].getHits().empty())
      {
        if (protein_significance_threshold_fraction == 0)
        {
          filtered_protein_identification = protein_identifications[i];
        }
        else
        {
          applied_filters.insert(String("Filtering by protein significance threshold fraction of ") + protein_significance_threshold_fraction + " ...\n");
          filter.filterIdentificationsByThreshold(protein_identifications[i], protein_significance_threshold_fraction, filtered_protein_identification);
        }

        if (sequences_file_name != "" && !no_protein_identifiers)
        {
          applied_filters.insert("Filtering by whitelisting protein accession from FASTA file ...\n");
          ProteinIdentification temp_identification = filtered_protein_identification;
          filter.filterIdentificationsByProteins(temp_identification, sequences, filtered_protein_identification);
        }

        if (protein_threshold_score != 0)
        {
          applied_filters.insert(String("Filtering by protein score > ") + protein_threshold_score + " ...\n");
          ProteinIdentification temp_identification = filtered_protein_identification;
          filter.filterIdentificationsByScore(temp_identification, protein_threshold_score, filtered_protein_identification);
        }

        if (best_n_protein_hits > 0)
        {
          applied_filters.insert("Filtering by best n protein hits ...\n");
          ProteinIdentification temp_identification = filtered_protein_identification;
          filter.filterIdentificationsByBestNHits(temp_identification, best_n_protein_hits, filtered_protein_identification);
        }

        if (!keep_unreferenced_protein_hits)
        {
          ProteinIdentification temp_identification = filtered_protein_identification;
          filter.removeUnreferencedProteinHits(temp_identification, filtered_peptide_identifications, filtered_protein_identification);
        }

        if (!(filtered_protein_identification.getHits().empty()))
        {
          filtered_protein_identifications.push_back(filtered_protein_identification);
        }
      }
      else
      {
        // copy the identifiers to the filtered protein ids
        filtered_protein_identifications.push_back(protein_identifications[i]);
      }
    }

    // check whether for each peptide identification identifier a corresponding protein id exists, if not add an empty one from the input file
    set<String> identifiers;
    for (vector<PeptideIdentification>::const_iterator it = filtered_peptide_identifications.begin(); it != filtered_peptide_identifications.end(); ++it)
    {
      identifiers.insert(it->getIdentifier());
    }

    for (set<String>::const_iterator it = identifiers.begin(); it != identifiers.end(); ++it)
    {
      // search for this identifier in filtered protein ids
      bool found(false);
      for (vector<ProteinIdentification>::const_iterator pit = filtered_protein_identifications.begin(); pit != filtered_protein_identifications.end(); ++pit)
      {
        if (*it == pit->getIdentifier())
        {
          found = true;
          break;
        }
      }

      if (!found)
      {
        // search this identifier in the protein id input
        found = false;
        ProteinIdentification new_prot_id;
        for (vector<ProteinIdentification>::const_iterator pit = protein_identifications.begin(); pit != protein_identifications.end(); ++pit)
        {
          if (*it == pit->getIdentifier())
          {
            new_prot_id = *pit;
            found = true;
            break;
          }
        }

        if (!found)
        {
          // this case means that the input file was not standard compatible
          writeLog_("Error: the identification run '" + *it + "' has no corresponding protein identification object!");
        }
        else
        {
          // just through away the protein hits
          new_prot_id.setHits(vector<ProteinHit>());
          filtered_protein_identifications.push_back(new_prot_id);
        }
      }
    }


    // print the filters used:
    for (std::set<String>::const_iterator it = applied_filters.begin(); it != applied_filters.end(); ++it)
    {
      LOG_INFO << *it;
    }

    // some stats
    LOG_INFO << "Peptide identifications remaining: " << filtered_peptide_identifications.size() << " / " << identifications.size() << "\n";
    LOG_INFO << "Protein identifications remaining: " << filtered_protein_identifications.size() << " / " << protein_identifications.size() << "\n";

    //-------------------------------------------------------------
    // writing output
    //-------------------------------------------------------------

    idXML_file.store(outputfile_name, filtered_protein_identifications, filtered_peptide_identifications);

    return EXECUTION_OK;
  }

};


int main(int argc, const char** argv)
{
  TOPPIDFilter tool;

  return tool.main(argc, argv);
}

/// @endcond
