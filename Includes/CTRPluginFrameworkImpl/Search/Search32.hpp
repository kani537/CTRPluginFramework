#ifndef CTRPLUGINFRAMEWORKIMPL_SEARCH32_HPP
#define CTRPLUGINFRAMEWORKIMPL_SEARCH32_HPP

#include "CTRPluginFrameworkImpl/Search/SearchBase.hpp"
#include "CTRPluginFrameworkImpl/System/Storage.hpp"

namespace CTRPluginFramework
{
    class Search32 : public Search
    {
        using StringVector = std::vector<std::string>;
        using u32Vector = std::vector<u32>;

    public:

        Search32(SearchParameters &parameters);
        ~Search32();

        // Read results
        void    ReadResults(u32 index, StringVector &addr, StringVector &newVal, StringVector &oldVal) override;


    private:

        Types32_u   _checkValue;

        // Return true if region is finished
        bool    FirstSearchSpecified(void) override;
        bool    FirstSearchUnknown(void) override;
        bool    SecondSearchSpecified(void) override;
        bool    SecondSearchUnknown(void) override;
        bool    SubsidiarySearchSpecified(void) override;
        bool    SubsidiarySearchUnknown(void) override;

        void    FirstSearchSpecifiedU8(u32 endAddress, SearchFlags compare, Results32 *result);
        void    FirstSearchSpecifiedU16(u32 endAddress, SearchFlags compare, Results32 *result);
        void    FirstSearchSpecifiedU32(u32 endAddress, SearchFlags compare, Results32 *result);
        void    FirstSearchSpecifiedFloat(u32 endAddress, SearchFlags compare, Results32 *result);

        void    SecondSearchSpecifiedU8(const std::vector<Results32> &data, SearchFlags compare, Results32WithOld *result);
        void    SecondSearchSpecifiedU16(const std::vector<Results32> &data, SearchFlags compare, Results32WithOld *result);
        void    SecondSearchSpecifiedU32(const std::vector<Results32> &data, SearchFlags compare, Results32WithOld *result);
        void    SecondSearchSpecifiedFloat(const std::vector<Results32> &data, SearchFlags compare, Results32WithOld *result);

        void    SecondSearchUnknownU8(const std::vector<u32> &data, SearchFlags compare, Results32WithOld *result);
        void    SecondSearchUnknownU16(const std::vector<u32> &data, SearchFlags compare, Results32WithOld *result);
        void    SecondSearchUnknownU32(const std::vector<u32> &data, SearchFlags compare, Results32WithOld *result);
        void    SecondSearchUnknownFloat(const std::vector<u32> &data, SearchFlags compare, Results32WithOld *result);

        void    SubsidiarySearchSpecifiedU8(const std::vector<Results32WithOld> &data, SearchFlags compare, Results32WithOld* result);
        void    SubsidiarySearchSpecifiedU16(const std::vector<Results32WithOld> &data, SearchFlags compare, Results32WithOld* result);
        void    SubsidiarySearchSpecifiedU32(const std::vector<Results32WithOld> &data, SearchFlags compare, Results32WithOld* result);
        void    SubsidiarySearchSpecifiedFloat(const std::vector<Results32WithOld> &data, SearchFlags compare, Results32WithOld* result);

        void    SubsidiarySearchUnknownU8(const std::vector<Results32WithOld> &data, SearchFlags compare, Results32WithOld* result);
        void    SubsidiarySearchUnknownU16(const std::vector<Results32WithOld> &data, SearchFlags compare, Results32WithOld* result);
        void    SubsidiarySearchUnknownU32(const std::vector<Results32WithOld> &data, SearchFlags compare, Results32WithOld* result);
        void    SubsidiarySearchUnknownFloat(const std::vector<Results32WithOld> &data, SearchFlags compare, Results32WithOld* result);



    };
}

#endif