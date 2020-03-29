using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ItemDBCGenerator
{
    class ItemRange
    {
        public UInt32[] Range { get; set; }
        public Constants.ItemClass ItemClass { get; set; }
        public Constants.WeaponSubclass WeaponSubClass { get; set; }
        public Constants.ArmorInventoryType ArmorInventoryType { get; set; }

        public ItemRange(UInt32[] range, Constants.ItemClass itemClass, Constants.ArmorInventoryType armorSubclass)
        {
            this.Range = range;
            this.ItemClass = itemClass;
            this.ArmorInventoryType = armorSubclass;
            this.WeaponSubClass = Constants.WeaponSubclass.NULL;
        }

        public ItemRange(UInt32[] range, Constants.ItemClass itemClass, Constants.WeaponSubclass weaponSubClass)
        {
            this.Range = range;
            this.ItemClass = itemClass;
            this.WeaponSubClass = weaponSubClass;
            this.ArmorInventoryType = Constants.ArmorInventoryType.NULL;
        }
    }
}
