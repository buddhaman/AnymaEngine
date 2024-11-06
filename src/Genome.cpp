#include "Genome.h"

Array<GeneNode>
CreateGenes(MemoryArena* arena, int n_genes)
{
    Array<GeneNode> genes = CreateArray<GeneNode>(arena, n_genes);
    GeneNode* node = nullptr;
    while(genes.size < n_genes)
    {
        node = genes.PushBack();
        node->hue_diff = RandomR32Debug(0.0f, 1.0f);
        node->sat_diff = RandomR32Debug(0.5f, 1.0f);
        node->brightness_diff = RandomR32Debug(0.5f, 1.0f);

        node->angle = RandomR32Debug(-0.1f, 0.1f);
        node->length = RandomR32Debug(1.0f, 5.0f);
        node->children = CreateArray<int>(arena, n_genes);

        if(genes.size != n_genes)
        {
            node->children.PushBack((int)(genes.size+1));
        }
    }
    return genes;
}