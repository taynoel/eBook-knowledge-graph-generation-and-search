eBook knowledge graph via MySQL (/graph_via_MySQL/)
=================================

Knowledge graph generation from eBook for/and information retrieval purposes. More info at https://arxiv.org/abs/1801.06664
Original ontology files (/BookOntology/wConcept/) and html files (/html/) are replaced due to copyright issues. 

The program needs FaCT++ and MySQL. Modify Makefile (/src/) accordingly

To train: ./run.sh

To use:
python ./graphWalk/graphWalkmysql.py 1 [A] [B,C,...]
A is the container type to filter the retrieved answers. B,C... are the query inputs
