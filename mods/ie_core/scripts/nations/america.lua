--------------------------------------------------------------
-- America
-- Initial nations
argentina = Nation:new{ ref_name = "argentina" }
argentina.name = _("Argentina")
Nation:register(argentina)
Nation:set_ideology(argentina, republic)
Nation:add_accepted_culture(argentina, argentinian)
Nation:add_client_hint(argentina, republic, _("Argentina"), 0xace3e1)

brazil = Nation:new{ ref_name = "brazil" }
brazil.name = _("Brazil")
Nation:register(brazil)
Nation:set_ideology(brazil, republic)
Nation:add_accepted_culture(brazil, brazilian)
Nation:add_client_hint(brazil, republic, _("Brazil"), 0x60c483)

gran_colombia = Nation:new{ ref_name = "gran_colombia" }
gran_colombia.name = _("Gran colombia")
Nation:register(gran_colombia)
Nation:set_ideology(gran_colombia, republic)
Nation:add_accepted_culture(gran_colombia, colombian)
Nation:add_accepted_culture(gran_colombia, venezuelan)
Nation:add_accepted_culture(gran_colombia, ecuadorian)
Nation:add_client_hint(gran_colombia, republic, _("Gran colombia"), 0xe6e6b5)

peru = Nation:new{ ref_name = "peru" }
peru.name = _("Peru")
Nation:register(peru)
Nation:set_ideology(peru, republic)
Nation:add_accepted_culture(peru, peruvian)
Nation:add_client_hint(peru, republic, _("Peru"), 0xe63d2e)

bolivia = Nation:new{ ref_name = "bolivia" }
bolivia.name = _("Bolivia")
Nation:register(bolivia)
Nation:set_ideology(bolivia, republic)
Nation:add_accepted_culture(bolivia, bolivian)
Nation:add_client_hint(bolivia, republic, _("Bolivia"), 0xded6ba)

paraguay = Nation:new{ ref_name = "paraguay" }
paraguay.name = _("Paraguay")
Nation:register(paraguay)
Nation:set_ideology(paraguay, republic)
Nation:add_accepted_culture(paraguay, paraguayan)
Nation:add_client_hint(paraguay, republic, _("Paraguay"), 0x808000)

chile = Nation:new{ ref_name = "chile" }
chile.name = _("Chile")
Nation:register(chile)
Nation:set_ideology(chile, republic)
Nation:add_accepted_culture(chile, chilean)
Nation:add_client_hint(chile, republic, _("Chile"), 0xad6dcf)

upca = Nation:new{ ref_name = "upca" }
upca.name = _("UPCA")
Nation:register(upca)
Nation:set_ideology(upca, republic)
Nation:add_accepted_culture(upca, mexican)
Nation:add_client_hint(upca, republic, _("UPCA"), 0x5bb2cf)

mexico = Nation:new{ ref_name = "mexico" }
mexico.name = _("Mexico")
Nation:register(mexico)
Nation:set_ideology(mexico, republic)
Nation:add_accepted_culture(mexico, mexican)
Nation:add_client_hint(mexico, republic, _("Mexico"), 0x5ac780)
Nation:add_client_hint(mexico, monarchy, _("Mexican Empire"), 0x5ac780)

united_states = Nation:new{ ref_name = "united_states" }
united_states.name = _("United states")
Nation:register(united_states)
Nation:set_ideology(united_states, democracy)
Nation:add_accepted_culture(united_states, british)
Nation:add_accepted_culture(united_states, yankee)
Nation:add_client_hint(united_states, democracy, _("United states"), 0x6ea5cc)

haiti = Nation:new{ ref_name = "haiti" }
haiti.name = _("Haiti")
Nation:register(haiti)
Nation:set_ideology(haiti, democracy)
Nation:add_accepted_culture(haiti, haitian)
Nation:add_client_hint(haiti, democracy, _("Haiti"), 0xb147cc)

-- Releasable nations
texas = Nation:new{ ref_name = "texas" }
texas.name = _("Texas")
Nation:register(texas)
Nation:add_accepted_culture(texas, yankee)
Nation:add_client_hint(texas, democracy, _("Texas"), 0x8be2f7)
--------------------------------------------------------------