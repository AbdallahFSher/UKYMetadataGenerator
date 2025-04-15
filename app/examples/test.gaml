graph {
  node Metadata {
    node DataCitation {
      attribute Abstract = "pending";
      attribute DateofDeposit = "pending";
      node Depositor {
        node [0] {
          attribute Name = "Bryan Maloney";
          attribute email = "bjma@uky.edu";
        }
      }
      attribute FundingAgency = "UK";
      attribute GrantNumber = "x";
      attribute Institution = "University of Kentucky";
      attribute Keywords = "Epilepsy drug discovery anti-inflammatory";
      node PrincipalInvestigator {
        node [0] {
          attribute Name = "Bjoern Bauer";
          attribute ORCID = "0000-0003-3867-2251";
          attribute email = "bjoern.bauer@uky.edu";
        }
        node [1] {
          attribute Name = "Anika Hartz";
          attribute ORCID = "0000-0001-9276-0688";
          attribute email = "anika.hartz@uky.edu";
        }
        node [2] {
          attribute Name = "Zhan";
          attribute ORCID = "";
          attribute email = "";
        }
      }
      attribute Title = "TEAMS Grant";
      attribute doi = "pending";
    }
    node Projects {
      node [0] {
        node Aims {
          node [0] {
            attribute Name = "Design, synthesis, & in vitro activity";
            node Workers {
              node [0] {
                attribute Name = "Zhan";
                attribute ORCID = "";
                attribute email = "";
              }
              node [1] {
                attribute Name = "Bjoern Bauer";
                attribute ORCID = "0000-0003-3867-2251";
                attribute email = "bjoern.bauer@uky.edu";
              }
            }
          }
        }
        attribute Name = "Novel mPGES Inhibitors";
        node Workers {
          node [0] {
            attribute Name = "Zhan";
            attribute ORCID = "";
            attribute email = "";
          }
          node [1] {
            attribute Name = "Bjoern Bauer";
            attribute ORCID = "0000-0003-3867-2251";
            attribute email = "bjoern.bauer@uky.edu";
          }
        }
      }
    }
    node Publications {
      node [0] {
        attribute Authors = "";
        attribute Journal = "";
        attribute Title = "";
        attribute Year = "";
      }
    }
  }
}
