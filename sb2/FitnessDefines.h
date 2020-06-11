//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
// was in Aedes.h
/*
//??
#define HAS_TWO_COPIES_OF_CONSTRUCT_ALPHA_EU_TYPE_1(genotype) ((genotype&32)&&(genotype&16))
#define HAS_ONE_COPY_OF_CONSTRUCT_ALPHA_EU_TYPE_1(genotype) (((genotype&32)&&(!(genotype&16)))||((!(genotype&32))&&(genotype&16)))
#define HAS_TWO_COPIES_OF_CONSTRUCT_BETA_EU_TYPE_1(genotype) ((genotype&8)&&(genotype&4))
#define HAS_ONE_COPY_OF_CONSTRUCT_BETA_EU_TYPE_1(genotype) (((genotype&8)&&(!(genotype&4)))||((!(genotype&8))&&(genotype&4)))
#define IS_NOT_VIABLE_EU_TYPE_1(genotype) (((genotype&12)&&(!(genotype&48)))||((!(genotype&12))&&((genotype&48))))
*/

#define HAS_CONSTRUCT_ALPHA_EU_TYPE_1(genotype) (genotype&48)
#define HAS_TWO_COPIES_OF_CONSTRUCT_ALPHA_EU_TYPE_1(genotype) ((genotype&32)&&(genotype&16))
#define HAS_CONSTRUCT_BETA_EU_TYPE_1(genotype) (genotype&12)
#define HAS_TWO_COPIES_OF_CONSTRUCT_BETA_EU_TYPE_1(genotype) ((genotype&8)&&(genotype&4))
#define IS_NOT_VIABLE_EU_TYPE_1(genotype) (((genotype&12)&&(!(genotype&48)))||((!(genotype&12))&&((genotype&48))))

#define HAS_TWO_COPIES_OF_CONSTRUCT_ALPHA_EU_TYPE_2(genotype) ((genotype&128)&&(genotype&64))
#define HAS_ONE_COPY_OF_CONSTRUCT_ALPHA_EU_TYPE_2(genotype) (((genotype&128)&&(!(genotype&64)))||((!(genotype&128))&&(genotype&64)))
#define HAS_TWO_COPIES_OF_CONSTRUCT_BETA_EU_TYPE_2(genotype) ((genotype&32)&&(genotype&16))
#define HAS_ONE_COPY_OF_CONSTRUCT_BETA_EU_TYPE_2(genotype) (((genotype&32)&&(!(genotype&16)))||((!(genotype&32))&&(genotype&16)))
#define HAS_TWO_COPIES_OF_CONSTRUCT_GAMMA_EU_TYPE_2(genotype) ((genotype&8)&&(genotype&4))
#define HAS_ONE_COPY_OF_CONSTRUCT_GAMMA_EU_TYPE_2(genotype) (((genotype&8)&&(!(genotype&4)))||((!(genotype&8))&&(genotype&4)))
#define IS_NOT_VIABLE_EU_TYPE_2(genotype) ((((genotype&192)||(genotype&48))&&(!(genotype&12)))||((!((genotype&192)||(genotype&48)))&&(genotype&12)))

#define HAS_TWO_COPIES_OF_CONSTRUCT_ALPHA_EU_TYPE_3(genotype) ((genotype&512)&&(genotype&256))
#define HAS_ONE_COPY_OF_CONSTRUCT_ALPHA_EU_TYPE_3(genotype) (((genotype&512)&&(!(genotype&256)))||((!(genotype&512))&&(genotype&256)))
#define HAS_TWO_COPIES_OF_CONSTRUCT_BETA_EU_TYPE_3(genotype) ((genotype&128)&&(genotype&64))
#define HAS_ONE_COPY_OF_CONSTRUCT_BETA_EU_TYPE_3(genotype) (((genotype&128)&&(!(genotype&64)))||((!(genotype&128))&&(genotype&64)))
#define HAS_TWO_COPIES_OF_CONSTRUCT_GAMMA_EU_TYPE_3(genotype) ((genotype&32)&&(genotype&16))
#define HAS_ONE_COPY_OF_CONSTRUCT_GAMMA_EU_TYPE_3(genotype) (((genotype&32)&&(!(genotype&16)))||((!(genotype&32))&&(genotype&16)))
#define HAS_TWO_COPIES_OF_CONSTRUCT_DELTA_EU_TYPE_3(genotype) ((genotype&8)&&(genotype&4))
#define HAS_ONE_COPY_OF_CONSTRUCT_DELTA_EU_TYPE_3(genotype) (((genotype&8)&&(!(genotype&4)))||((!(genotype&8))&&(genotype&4)))
#define IS_NOT_VIABLE_EU_TYPE_3(genotype) ((((genotype&768)||(genotype&192)||(genotype&48))&&(!(genotype&12)))||((!((genotype&768)||(genotype&192)||(genotype&48)))&&(genotype&12)))

#define HAS_TWO_COPIES_OF_CONSTRUCT_ALPHA_EU_TYPE_4(genotype) ((genotype&512)&&(genotype&256))
#define HAS_ONE_COPY_OF_CONSTRUCT_ALPHA_EU_TYPE_4(genotype) (((genotype&512)&&(!(genotype&256)))||((!(genotype&512))&&(genotype&256)))
#define HAS_TWO_COPIES_OF_CONSTRUCT_BETA_EU_TYPE_4(genotype) ((genotype&128)&&(genotype&64))
#define HAS_ONE_COPY_OF_CONSTRUCT_BETA_EU_TYPE_4(genotype) (((genotype&128)&&(!(genotype&64)))||((!(genotype&128))&&(genotype&64)))
#define HAS_TWO_COPIES_OF_CONSTRUCT_GAMMA_EU_TYPE_4(genotype) ((genotype&32)&&(genotype&16))
#define HAS_ONE_COPY_OF_CONSTRUCT_GAMMA_EU_TYPE_4(genotype) (((genotype&32)&&(!(genotype&16)))||((!(genotype&32))&&(genotype&16)))
#define HAS_TWO_COPIES_OF_CONSTRUCT_DELTA_EU_TYPE_4(genotype) ((genotype&8)&&(genotype&4))
#define HAS_ONE_COPY_OF_CONSTRUCT_DELTA_EU_TYPE_4(genotype) (((genotype&8)&&(!(genotype&4)))||((!(genotype&8))&&(genotype&4)))
#define IS_NOT_VIABLE_EU_TYPE_4(genotype) ((((genotype&768)||(genotype&192))&&(!((genotype&48)||(genotype&12))))||((!((genotype&768)||(genotype&192)))&&((genotype&48)||(genotype&12))))

